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

#include <pin_definition.hpp>

namespace Placer {

struct MacroDefinition {
    std::string name;
    std::string id;

    size_t width;
    size_t height;

    std::vector<PinDefinition> pin_definitions;
};

} /* namespace Placer */

#endif /* MACRO_DEFINITION_HPP */
