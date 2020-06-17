//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : macro_defintion.hpp
//
// Date         : 05. May 2020
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Macro Definition
//==================================================================
#ifndef MACRO_DEFINITION_HPP
#define MACRO_DEFINITION_HPP

#include <string>
#include <object.hpp>

#include <pin_definition.hpp>

namespace Placer {

struct MacroDefinition {
    
    MacroDefinition()
    {
        lx = 0;
        ly = 0;
        is_placed = false;
    }

    std::string name;
    std::string id;
    bool is_placed;

    size_t width;
    size_t height;

    size_t lx;
    size_t ly;
    eOrientation orientation;

    std::vector<PinDefinition> pin_definitions;
};

} /* namespace Placer */

#endif /* MACRO_DEFINITION_HPP */
