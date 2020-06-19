#include "optlong.h"
#include <sstream>
#include <string.h>
#include <stdlib.h>


Options &Options::define_string(char onechar, std::string const &name, std::string const &dflt, std::string const &doc) {
    if (docs.find(name) != docs.end()) {
        errors.push_back("Option " + name + " defined twice.");
        return *this;
    }
    if (onechar != 0 && onechars.find(onechar) != onechars.end()) {
        errors.push_back("Short option " + std::string(&onechar, 1) + " defined twice.");
        return *this;
    }
    if (onechar != 0) {
        onechars[onechar] = name;
        onecharsRev[name] = onechar;
    }
    docs[name] = doc;
    strings[name] = dflt;
    return *this;
}

Options &Options::define_double(char onechar, std::string const &name, double dflt, std::string const &doc) {
    if (docs.find(name) != docs.end()) {
        errors.push_back("Option " + name + " defined twice.");
        return *this;
    }
    if (onechar != 0 && onechars.find(onechar) != onechars.end()) {
        errors.push_back("Short option " + std::string(&onechar, 1) + " defined twice.");
        return *this;
    }
    if (onechar != 0) {
        onechars[onechar] = name;
        onecharsRev[name] = onechar;
    }
    docs[name] = doc;
    doubles[name] = dflt;
    return *this;
}

Options &Options::define_int64(char onechar, std::string const &name, int64_t dflt, std::string const &doc) {
    if (docs.find(name) != docs.end()) {
        errors.push_back("Option " + name + " defined twice.");
        return *this;
    }
    if (onechar != 0 && onechars.find(onechar) != onechars.end()) {
        errors.push_back("Short option " + std::string(&onechar, 1) + " defined twice.");
        return *this;
    }
    if (onechar != 0) {
        onechars[onechar] = name;
        onecharsRev[name] = onechar;
    }
    docs[name] = doc;
    int64s[name] = dflt;
    return *this;
}

Options &Options::define_flag(char onechar, std::string const &name, std::string const &doc) {
    if (docs.find(name) != docs.end()) {
        errors.push_back("Option " + name + " defined twice.");
        return *this;
    }
    if (onechar != 0 && onechars.find(onechar) != onechars.end()) {
        errors.push_back("Short option " + std::string(&onechar, 1) + " defined twice.");
        return *this;
    }
    if (onechar != 0) {
        onechars[onechar] = name;
        onecharsRev[name] = onechar;
    }
    docs[name] = doc;
    flags[name] = false;
    return *this;
}

std::string Options::get_string(std::string const &name) {
    auto sp = strings.find(name);
    if (sp == strings.end()) {
        throw std::invalid_argument("Asked for string " + name);
    }
    return sp->second;
}

double Options::get_double(std::string const &name) {
    auto dp = doubles.find(name);
    if (dp == doubles.end()) {
        throw std::invalid_argument("Asked for double " + name);
    }
    return dp->second;
}

int64_t Options::get_int64(std::string const &name) {
    auto ip = int64s.find(name);
    if (ip == int64s.end()) {
        throw std::invalid_argument("Asked for integer " + name);
    }
    return ip->second;
}

bool Options::get_flag(std::string const &name) {
    auto fp = flags.find(name);
    if (fp == flags.end()) {
        throw std::invalid_argument("Asked for flag " + name);
    }
    return fp->second;
}


std::string Options::help(bool withDefaults) const {
    std::stringstream ss;
    for (auto const &doc : docs) {
        auto ocr = onecharsRev.find(doc.first);
        if (ocr != onecharsRev.end()) {
            ss << "-" << ocr->second << " ";
        } else {
            ss << "   ";
        }
        ss << "--" << doc.first;
        if (doc.first.size() < 15) {
            static char const spaces[] = "                ";
            char const *pad = &spaces[doc.first.size()];
            ss << pad;
        }
        ss << "  ";
        if (withDefaults) {
            ss << "(default ";
            auto sd = strings.find(doc.first);
            if (sd != strings.end()) {
                ss << '"' << sd->second << '"';
            }
            auto dd = doubles.find(doc.first);
            if (dd != doubles.end()) {
                ss << dd->second;
            }
            auto id = int64s.find(doc.first);
            if (id != int64s.end()) {
                ss << id->second;
            }
            ss << ")" << std::endl;
            ss << "                       " << doc.second << std::endl;
        } else {
            ss << doc.second << std::endl;
        }
    }
    return ss.str();
}

bool Options::parse(int *argc, char const **argv) {
    bool restfiles = false;
    for (int i = 1; i < *argc; ++i) {
        if (!strcmp(argv[i], "--")) {
            restfiles = true;
            continue;
        }
        if (restfiles || argv[i][0] != '-' || !strcmp(argv[i], "-")) {
            files.push_back(argv[i]);
            continue;
        }
        if (argv[i][1] == '-') {
            //  long option
            if (!parse_option(argc, argv, &argv[i][2], i)) {
                errors.push_back("Option --" + std::string(&argv[i][2]) + " is not recognized.");
            } else {
                optsSpecified.insert(argv[i]);
            }
        } else {
            //  short option
            for (char const *opts = argv[i]+1; *opts; opts++) {
                auto so = onechars.find(*opts);
                if (so == onechars.end()) {
                    errors.push_back("Option -" + std::string(opts, 1) + " is not recognized.");
                } else {
                    parse_option(argc, argv, so->second, i);
                    optsSpecified.insert(so->second);
                }
            }
        }
    }
    return errors.empty();
}

bool Options::operator!() const {
    return !errors.empty();
}

//  Return true if the option name is recognized (even if the value is bad)
bool Options::parse_option(int *argc, char const **argv, std::string const &name, int &i) {
    if (optsSpecified.find(name) != optsSpecified.end()) {
        errors.push_back("Option --" + name + " was specified twice.");
        return true;
    }
    auto sop = strings.find(name);
    if (sop != strings.end()) {
        if (argv[i+1] == NULL) {
            errors.push_back("Option --" + name + " requires a string value.");
            return true;
        }
        i++;
        strings[name] = argv[i];
        return true;
    }
    auto dop = doubles.find(name);
    if (dop != doubles.end()) {
        if (argv[i+1] == NULL) {
            errors.push_back("Option --" + name + " requires a double value.");
            return true;
        }
        i++;
        doubles[name] = parse_double(name, argv[i]);
        return true;
    }
    auto inp = int64s.find(name);
    if (inp != int64s.end()) {
        if (argv[i+1] == NULL) {
            errors.push_back("Option --" + name + " requires an integer value.");
            return true;
        }
        i++;
        int64s[name] = parse_int64(name, argv[i]);
        return true;
    }
    auto flp = flags.find(name);
    if (flp != flags.end()) {
        flags[name] = true;
        return true;
    }
    //  not recognized
    return false;
}

double Options::parse_double(std::string const &name, std::string const &value) {
    char *oot = NULL;
    double d = strtod(value.c_str(), &oot);
    if (oot == value.c_str() || !oot) {
        errors.push_back("Option --" + name + " doesn't recognize " + value + " as a valid double.");
    }
    return d;
}

int64_t Options::parse_int64(std::string const &name, std::string const &value) {
    char *oot = NULL;
    int64_t ll = strtoll(value.c_str(), &oot, 0);
    if (oot == value.c_str() || !oot) {
        errors.push_back("Option --" + name + " doesn't recognize " + value + " as a valid integer.");
    }
    return ll;
}


