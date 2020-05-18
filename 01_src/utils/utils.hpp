//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : utils.hpp
//
// Date         : 07.January 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Utility Class
//==================================================================
#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>
#include <fstream>
#include <chrono>
#include <unistd.h>
#include <sys/utsname.h>

#include <boost/filesystem.hpp>
#include <boost/process.hpp>

namespace Placer::Utils {

class Utils {
public:

    static int system_execute(std::string const & bin,
                              std::vector<std::string> & args,
                              std::string const & output,
                              int & pid,
                              bool wait_for_termination);

    static int system_execute(std::string const & bin,
                              std::vector<std::string> & args,
                              std::string const & output,
                              bool wait_for_termination);

    static std::vector<std::string> tokenize(std::string const & str,
                                             std::string const & delimiters);

    static std::string get_bash_string_blink_red(std::string const & str);

    static std::string get_bash_string_cyan(std::string const & str);

    static std::string get_bash_string_orange(std::string const & str);

    static std::string get_bash_string_purple(const std::string& str);

    static std::string get_bash_string_red(std::string const & str);

    static std::string get_bash_string_green(std::string const & str);

    static std::string get_bash_string_blue(std::string const & str);

    static std::string get_bash_string_blink_purple(std::string const & str);

    static std::string get_base_path();

    static std::string get_current_time();

    static std::string get_current_user();

    static std::string get_plattform();

    template<typename T>
    static T gcd(std::vector<T> const & input);
};

}

#include <utils.inl>

#endif /* UTILS_HPP */
