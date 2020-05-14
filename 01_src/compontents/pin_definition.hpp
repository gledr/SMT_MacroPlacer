//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : pin_defintion.hpp
//
// Date         : 14. May 2020
// Compiler     : gcc version 9.3.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Preliminary Pin Definition
//==================================================================
#ifndef PIN_DEFINITION_HPP
#define PIN_DEFINITION_HPP

#include <string>

namespace Placer {

struct PinDefinition {
    std::string parent;
    std::string name;
    std::string direction;
};

}

#endif /* PIN_DEFINITION_HPP */
