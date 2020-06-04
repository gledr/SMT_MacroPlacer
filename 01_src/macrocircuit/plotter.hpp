//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : plotter.hpp
//
// Date         : 04. June 2020
// Compiler     : gcc version 10.1.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : LEF/DEF MacroCircuit Plotter
//==================================================================
#ifndef PLOTTER_HPP
#define PLOTTER_HPP

#include <object.hpp>
#include <components.hpp>
#include <layout.hpp>

namespace Placer {

class Plotter: public virtual Object{
public:
    Plotter();

    virtual ~Plotter();
    
    void set_data(std::vector<Terminal*> const & terminals,
                  Layout* const layout);

private:
    std::vector<Terminal*> m_terminals;
    Layout* m_layout;

};

} /* namespace Placer */

#endif /* PLOTTER_HPP */
