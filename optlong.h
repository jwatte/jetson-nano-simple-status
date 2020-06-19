#if !defined(optlong_h)
#define optlong_h

#include <map>
#include <set>
#include <string>
#include <vector>
#include <stdint.h>

struct Options {

    //  define the options you recognize
    //  onechar can be 0 for options that don't have it
    Options &define_string(char onechar, std::string const &name, std::string const &dflt, std::string const &doc);
    Options &define_double(char onechar, std::string const &name, double dflt, std::string const &doc);
    Options &define_int64(char onechar, std::string const &name, int64_t dflt, std::string const &doc);
    //  All flags are default to off and will turn on when specified
    Options &define_flag(char onechar, std::string const &name, std::string const &doc);

    //  Read out the values you got after calling parse().
    //  This will throw exceptions if you ask for a flag that's not defined.
    //  To know which files were specified, look at the 'files' vector.
    std::string get_string(std::string const &name);
    double get_double(std::string const &name);
    int64_t get_int64(std::string const &name);
    bool get_flag(std::string const &name);

    //  You can also just look at these maps
    std::map<std::string, std::string> strings;
    std::map<std::string, double> doubles;
    std::map<std::string, int64_t> int64s;
    std::map<std::string, bool> flags;
    std::vector<std::string> files;

    //  you may have gotten errors, or want to check which options
    //  were actually given
    std::vector<std::string> errors;
    std::set<std::string> optsSpecified;

    //  parse the command line, returns true on no error
    bool parse(int *argc, char const **argv);
    //  returns true if there's an error
    bool operator!() const;
    //  return a string suitable for explaining options to the user
    std::string help(bool withDefaults = false) const;

    //  internal helpers -- but forcing "private" is dumb. Use them if you have to.
    bool parse_option(int *argc, char const **argv, std::string const &name, int &i);
    double parse_double(std::string const &name, std::string const &value);
    int64_t parse_int64(std::string const &name, std::string const &value);

    std::map<std::string, std::string> docs;
    std::map<char, std::string> onechars;
    std::map<std::string, char> onecharsRev;

};

#endif  //  optlong_h
