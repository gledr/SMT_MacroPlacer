//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : exception.cpp
//
// Date         : 19.May 2020
// Compiler     : gcc version 9.3.0 (GCC)
// Copyright    : Johannes Kepler University
// Description  : Exception/Assertion Classes
//==================================================================
#include "exception.hpp"

using namespace Placer::Utils;


/**
 * @brief Type Safe Function called by Macro to check Assertion
 * 
 * @param file File where Assertion Failed
 * @param line Line Number where Assertion Failed
 */
void RunTimeCheck::__assertion_check__(std::string const & file, size_t const line)
{
    throw PlacerException("Assertion Violated (" + file + ":" + std::to_string(line) + ")");
}

/**
 * @brief Type Safe Function Called by Macro to check Null Pointer
 * 
 * @param file File where Null Pointer Occured
 * @param line Line where Null Pointer was found
 */
void RunTimeCheck::__nullpointer_check__(std::string const & file, size_t const line)
{
    throw PlacerNullPointerException("Placer NullPointerException! " + file + ":" + std::to_string(line));
}

/**
 * @brief Type Safe Function Called by Macro to Report Not Implemented Functions
 * 
 * @param file File where not implemented feature was found
 * @param line Line Number where not implemented feature was found
 */
void RunTimeCheck::__notimplemented_check__(std::string const & file, size_t const line)
{
    throw PlacerNotImplemented("Feature Not Implemented! " + file + ":" + std::to_string(line));
}

/**
 * @brief Type Safe Function called by Macro to Report Not Supported Functionality
 * 
 * @param msg Error Message
 * @param file File where Macro was called
 * @param line Line where Macro was called
 */
void RunTimeCheck::__notsupported_check__(std::string const & msg, std::string const & file, size_t const line)
{
    std::stringstream msg_str;
    msg_str << msg << "(" << file << ":" << line << ")";
    throw PlacerNotSupported(msg_str.str());
}
