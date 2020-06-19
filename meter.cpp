#include "meter.h"
#include <string.h>
#include <FL/fl_draw.H>
#include <math.h>
#include <stdio.h>

Fl_Meter::Fl_Meter(int l, int t, int w, int h, char const *lab) :
    Fl_Widget(l, t, w, h, lab),
    lineColor_(FL_GREEN),
    minRange_(0),
    maxRange_(1)
{
    color(FL_DARK3);
    labelcolor(FL_WHITE);
    align(FL_ALIGN_CENTER | FL_ALIGN_TOP | FL_ALIGN_INSIDE);
}

Fl_Meter::~Fl_Meter()
{
}

void Fl_Meter::copy_data(float const *d, size_t count)
{
    data_.resize(count);
    memcpy(&data_[0], d, count*sizeof(float));
    calc_range();
}

void Fl_Meter::calc_range()
{
    if (data_.empty()) {
        minRange_ = 0;
        maxRange_ = 1;
        return;
    }
    minRange_ = maxRange_ = data_[0];
    for (auto const f : data_) {
        if (f < minRange_) {
            minRange_ = f;
        }
        if (f > maxRange_) {
            maxRange_ = f;
        }
    }
    if (maxRange_ - minRange_ < 1.0f) {
        float mr = minRange_ + maxRange_;
        minRange_ = (mr-1)*0.5f;
        maxRange_ = (mr+1)*0.5f;
    }
    if ((minRange_ >= 0) && (minRange_ < (maxRange_ * 0.5f))) {
        minRange_ = 0;
    } else if (minRange_ > 1.0f) {
        minRange_ -= 1.0f;
        maxRange_ += 1.0f;
    }
    redraw();
}

void Fl_Meter::range(float min, float max)
{
    minRange_ = min;
    maxRange_ = max;
    if (maxRange_ <= minRange_) {
        maxRange_ = minRange_+1;
    }
    redraw();
}

void Fl_Meter::linecolor(Fl_Color c)
{
    lineColor_ = c;
}

static void range_format(float v, char *buf) {
    v = fabsf(v);
    char const *fmt = "%.2e";
    float mul = 1;
    if (v < 1.0f) {
        fmt = "%.3f";
    } else if (v < 10.0f) {
        fmt = "%.2f";
    } else if (v < 100.0f) {
        fmt = "%.1f";
    } else if (v < 1e3f) {
        fmt = "%.0f";
    } else if (v < 1e4f) {
        fmt = "%.2fk";
        mul = 1e-3;
    } else if (v < 1e5f) {
        fmt = "%.1fk";
        mul = 1e-3;
    } else if (v < 1e6f) {
        fmt = "%.0fk";
        mul = 1e-3;
    } else if (v < 1e7f) {
        fmt = "%.2fM";
        mul = 1e-6;
    } else if (v < 1e8f) {
        fmt = "%.1fM";
        mul = 1e-6;
    } else if (v < 1e9f) {
        fmt = "%.0fM";
        mul = 1e-6;
    } else if (v < 1e10f) {
        fmt = "%.2fG";
        mul = 1e-9;
    } else if (v < 1e11f) {
        fmt = "%.1fG";
        mul = 1e-9;
    } else if (v < 1e12f) {
        fmt = "%.0fG";
        mul = 1e-9;
    } else if (v < 1e13f) {
        fmt = "%.2fT";
        mul = 1e-12;
    } else if (v < 1e14f) {
        fmt = "%.1fT";
        mul = 1e-12;
    } else if (v < 1e15f) {
        fmt = "%.0fT";
        mul = 1e-12;
    } else if (v < 1e16f) {
        fmt = "%.2fP";
        mul = 1e-15;
    }
    sprintf(buf, fmt, v * mul);
}

void Fl_Meter::draw()
{
    float left = float(x());
    float top = float(y());
    float right = left + float(w());
    float bottom = top + float(h());
    if (bottom <= top || right <= left) {
        return;
    }
    fl_rectf(x(), y(), w(), h(), color());
    draw_box();
    fl_color(lineColor_);
    float minRange = minRange_;
    float maxRange = maxRange_;
    char buf[50];
    range_format(minRange, buf);
    fl_draw(buf, x(), y(), w(), h(), FL_ALIGN_LEFT | FL_ALIGN_BOTTOM | FL_ALIGN_INSIDE, NULL, 0);
    range_format(maxRange, buf);
    fl_draw(buf, x(), y(), w(), h(), FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE, NULL, 0);
    auto proj = [top, bottom, minRange, maxRange](float v) -> float {
        return bottom + (top - bottom) * (v - minRange) / (maxRange - minRange);
    };
    if (!data_.empty()) {
        float p = proj(data_[0]);
        if (data_.size() == 1) {
            fl_line(left, p, right, p);
        } else if (data_.size() > 1) {
            size_t n = data_.size();
            float m = float(n)-1;
            float xx = left;
            for (size_t i = 1; i != n; ++i) {
                float np = proj(data_[i]);
                float nx = (left*(m-i) + right*i)/m;
                fl_line(xx, p, nx, np);
                xx = nx;
                p = np; // proof!
            }
        } //  else draw nothing
    }
    draw_label();
}



