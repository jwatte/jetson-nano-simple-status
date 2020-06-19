#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Multiline_Output.H>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <atomic>

#include "meter.h"
#include "optlong.h"

Fl_Window *window;
Fl_Multiline_Output *output;
Fl_Meter *cpuMeter;
Fl_Meter *ramMeter;
Fl_Meter *netMeter;
Fl_Meter *tempMeter;

pthread_t thread;
pthread_mutex_t textMutex = PTHREAD_MUTEX_INITIALIZER;

char thetext[10000] = "";

std::atomic_bool changed;
std::atomic<float> cpuValue;
std::atomic<float> ramValue;
std::atomic<float> netValue;
std::atomic<float> tempValue;

std::string netInterface("eth0");

static void read_cpu_used() {
    FILE *f = fopen("/proc/loadavg", "r");
    cpuValue = 0;
    if (f != NULL) {
        float val = 0;
        fscanf(f, "%f", &val);
        fclose(f);
        cpuValue = val;
    }
}

static void read_mem_active() {
    FILE *f = fopen("/proc/meminfo", "r");
    if (f == NULL) {
        return;
    }
    char line[1025];
    int active = 0;
    int total = 0;
    while (true) {
        line[0] = 0;
        if (!fgets(line, 1024, f)) {
            break;
        }
        line[1024] = 0;
        if (1 == sscanf(line, "MemTotal: %d", &total)) {
            if (active != 0) {
                break;
            }
        }
        if (1 == sscanf(line, "Active: %d", &active)) {
            if (total != 0) {
                break;
            }
        }
    }
    if (total > 0) {
        //  percent
        ramValue = float(active)/float(total)*100.0f;
    }
    fclose(f);
}

int64_t lastNetIn;
int64_t lastNetOut;
char nettext[10241];

static void read_net_user(float secondsSinceRead) {
    int netInPos = 0;
    int netOutPos = 0;
    FILE *f = fopen("/proc/net/netstat", "r");
    if (f == NULL) {
        return;
    }
    long l = fread(nettext, 1, 10240, f);
    fclose(f);
    if (l > 0) {
        nettext[l] = 0;
        char *str = strstr(nettext, "IpExt:");
        char *begin = str;
        if (str != NULL) {
            for (int n = 0; *str != '\n'; ++str) {
                if (*str == ' ') {
                    *str = 0;
                    if (!strcmp(begin, "InOctets")) {
                        netInPos = n;
                    }
                    if (!strcmp(begin, "OutOctets")) {
                        netOutPos = n;
                    }
                    begin = str+1;
                    n++;
                }
            }
            str = strstr(str, "IpExt:");
        }
        int64_t netin = 0;
        int64_t netout = 0;
        if (str != NULL) {
            for (int n = 0; *str && (*str != '\n'); ++str) {
                if (*str == ' ') {
                    *str = 0;
                    if (n == netInPos) {
                        sscanf(begin, "%ld", &netin);
                    }
                    if (n == netOutPos) {
                        sscanf(begin, "%ld", &netout);
                    }
                    begin = str+1;
                    n++;
                }
            }
        }
        //  -- read every 10 seconds
        netValue = float(double((netin - lastNetIn) + (netout - lastNetOut)) / secondsSinceRead);
        if (lastNetIn == 0 && lastNetOut == 0) {
            //  anchor
            netValue = 0;
        }
        lastNetIn = netin;
        lastNetOut = netout;
    }
}

static void read_temp() {
    //  zone1 is nano CPU temp -- zone2 is nano GPU
    FILE *f = fopen("/sys/devices/virtual/thermal/thermal_zone1/temp", "r");
    if (f == NULL) {
        return;
    }
    char buf[101];
    long l = fread(buf, 1, 100, f);
    fclose(f);
    if (l > 0) {
        buf[l] = 0;
        int item = 0;
        sscanf(buf, "%d", &item);
        tempValue = item / 1000.0f;
    }
}

#define SLEEP_TIME 5

static void *poll_func(void *) {
    char buf[sizeof(thetext)+1];
    while (true) {
        long l = -1;
        FILE *f = popen(("/sbin/ip addr show dev '" + netInterface + "'").c_str(), "r");
        if (f != NULL) {
            l = fread(buf, 1, sizeof(thetext), f);
            fclose(f);
            f = NULL;
        }
        if (l < 0) {
            sprintf(buf, "error getting IP address: %s", strerror(errno));
        } else {
            pthread_mutex_lock(&textMutex);
            memcpy(thetext, buf, l);
            thetext[l] = 0;
            pthread_mutex_unlock(&textMutex);
        }
        read_cpu_used();
        read_mem_active();
        read_net_user(SLEEP_TIME);
        read_temp();
        changed = true;
        sleep(SLEEP_TIME);
    }
}

static void push(Fl_Meter *m, float v) {
    m->data_.push_back(v);
    if (m->data_.size() > size_t(m->w()+1)) {
        m->data_.erase(m->data_.begin());
    }
    m->calc_range();    //  calc_range() calls redraw()
}

static void check_new_data(void *) {
    if (changed) {
        changed = false;
        output->value(thetext);
        output->redraw();
        push(cpuMeter, cpuValue);
        push(ramMeter, ramValue);
        push(netMeter, netValue);
        push(tempMeter, tempValue);
    }
    Fl::add_timeout(1.0f, &check_new_data, 0);
}


int main(int argc, char const **argv) {

    Options op;
    op.define_string('i', "interface", netInterface, "which network interface to look for");
    op.define_int64('x', "xpos", 64, "horizontal coordinate to put the window");
    op.define_int64('y', "ypos", 32, "vertical coordinate to put the window");
    op.define_flag('?', "help", "show options");
    if (!op.parse(&argc, argv)) {
        for (auto const &e : op.errors) {
            fprintf(stderr, "error: %s\n", e.c_str());
        }
        return 1;
    }
    if (op.flags["help"]) {
        fprintf(stderr, "help:\n%s", op.help(true).c_str());
        return 1;
    }
    if (!op.files.empty()) {
        fprintf(stderr, "Extra arguments not recognized.\n");
        return 1;
    }
    netInterface = op.get_string("interface");
    int xpos = (int)op.get_int64("xpos");
    int ypos = (int)op.get_int64("ypos");

    window = new Fl_Double_Window(xpos, ypos, 1280, 320, "Jetson Nano Status");
    window->color(FL_DARK3);

    output = new Fl_Multiline_Output(0, 0, 1280, 128, thetext);
    output->value("Fetching status...\n");
    output->textfont(FL_COURIER);
    output->textsize(16);
    output->color(FL_DARK3);
    output->textcolor(FL_GREEN);
    output->box(FL_FLAT_BOX);

    cpuMeter = new Fl_Meter(10, 138, 308, 172, "Load");
    cpuMeter->linecolor(FL_YELLOW);
    cpuMeter->labelcolor(FL_YELLOW);
    ramMeter = new Fl_Meter(328, 138, 308, 172, "RAM%");
    ramMeter->linecolor(FL_RED);
    ramMeter->labelcolor(FL_RED);
    netMeter = new Fl_Meter(646, 138, 308, 172, "Net/s");
    netMeter->linecolor(FL_CYAN);
    netMeter->labelcolor(FL_CYAN);
    tempMeter = new Fl_Meter(964, 138, 308, 172, "Temp C");
    tempMeter->linecolor(FL_MAGENTA);
    tempMeter->labelcolor(FL_MAGENTA);

    window->end();

    //  Dummy up a command line for FLTK
    char jss[] = "jetson-nano-simple-status";
    char *buf[] = { jss, NULL };
    window->show(1, buf);
    Fl::add_timeout(1.0f, &check_new_data, 0);
    pthread_create(&thread, NULL, &poll_func, NULL);
    return Fl::run();
}
