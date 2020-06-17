//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : terminal_definition.hpp
//
// Date         : 05. May 2020
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : SoC Terminal Definition
//==================================================================
#ifndef TERMINAL_DEFINITION
#define TERMINAL_DEFINITION

#include <string>

namespace Placer {

struct TerminalDefinition {

    TerminalDefinition()
    {
        is_free = false;
    }
    std::string name;

    size_t pos_x;
    size_t pos_y;
    bool is_free;
};

} /* namespace Placer */

#endif /* TERMINAL_DEFINITION */
