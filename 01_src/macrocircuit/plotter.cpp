//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : plotter.cpp
//
// Date         : 04. June 2020
// Compiler     : gcc version 10.1.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : LEF/DEF MacroCircuit Plotter
//==================================================================
#include "plotter.hpp"

using namespace Placer;

Plotter::Plotter():
    Object()
{
}

Plotter::~Plotter()
{
}

void Plotter::set_data(std::vector<Terminal*> const & terminals,
                       Layout *const layout)
{
    nullpointer_check(layout);
    
    m_layout = layout;
    m_terminals = terminals;
}
