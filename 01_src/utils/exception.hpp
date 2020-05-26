//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : exception.hpp
//
// Date         : 19.May 2020
// Compiler     : gcc version 9.3.0 (GCC)
// Copyright    : Johannes Kepler University
// Description  : Exception/Assertion Classes
//==================================================================
#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include <exception>
#include <iostream>
#include <sstream>

namespace Placer::Utils {

/**
 * @class RunTimeCheck
 * @brief Type Safe Functions to be called by Macros
 */
class RunTimeCheck {
public:
    static void __nullpointer_check__(std::string const & file,
                                      size_t const line);
    static void __assertion_check__(std::string const & file,
                                    size_t const line);
    static void __notimplemented_check__(std::string const & file,
                                         size_t const line);
    static void __notsupported_check__(std::string const & msg,
                                       std::string const & file,
                                       size_t const line);
};

} /* namespace Placer::Utils */

#define nullpointer_check(expr)                                                         \
    if(!expr)                                                                           \
    {                                                                                   \
        using Placer::Utils::RunTimeCheck;                                              \
        RunTimeCheck::__nullpointer_check__(__FILE__, __LINE__);                        \
    }

#define assertion_check(expr)                                                           \
    if(!static_cast<bool>(expr))                                                        \
    {                                                                                   \
        Placer::Utils::RunTimeCheck::__assertion_check__(__FILE__, __LINE__);           \
    }

#define notimplemented_check()                                                          \
    Placer::Utils::RunTimeCheck::__notimplemented_check__(__FILE__, __LINE__);

#define notsupported_check(expr)                                                        \
    Placer::Utils::RunTimeCheck::__notsupported_check__(expr, __FILE__, __LINE__);

namespace Placer::Utils {

/**
 * @class PlacerNotImplemented
 * @brief Not Implemented Exception Type
 */ 
class PlacerNotImplemented: public std::logic_error {
public:
    /**
     * @brief Constructor
     * 
     * @param what Error Message
     */
    PlacerNotImplemented(std::string const & what):
        std::logic_error(what) 
    {
        std::cerr << this->what() << std::endl;
        exit (0);
    }
};

/**
 * @class PlacerNotSupported
 * @brief Not Supported Exception Type
 */ 
class PlacerNotSupported: public std::logic_error {
public:
    /**
     * @brief Constructor 
     * 
     * @param what Error Message
     */
    PlacerNotSupported(std::string const & what):
        std::logic_error ("PlacerNotSupported: " + what) 
    {
        std::cerr << this->what() << std::endl;
        exit (0);
    }
};

/**
 * @class PlacerException
 * @brief Basic Exception Type
 */ 
class PlacerException: public std::logic_error {
public:
    /**
     * @brief Constructor
     * 
     * @param what Error Message
     */
    PlacerException (std::string const & what):
        std::logic_error("PlacerException: " + what)
    {
        std::cerr << this->what() << std::endl;
        exit (0);
    }
};

/**
 * @class PlacerNullPointerException
 * @brief Placer Null Pointer Exception Type
 */ 
class PlacerNullPointerException: public std::exception {
public:
    /**
     * @brief Constructor
     * 
     * @param what Error Message
     */
    PlacerNullPointerException (std::string const & what):
        std::exception() 
    {
        std::cerr << std::endl;
        std::cerr << what << std::endl;
        exit(-1);
    }
};

} /* namespace Placer::Utils */

#endif /* EXCEPTIONS_HPP */
