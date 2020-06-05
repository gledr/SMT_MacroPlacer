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

#include <matplotlibcpp.h>

namespace Placer {

class Plotter: public virtual Object{
public:
    Plotter();

    virtual ~Plotter();
    
    void set_data(std::vector<Terminal*> const & terminals,
                  std::vector<Component*> const & components,
                  size_t const soultion_id,
                  Layout* const layout);
    
    void run();

private:
    std::vector<Terminal*> m_terminals;
    std::vector<Component*> m_components;
    Layout* m_layout;
    size_t m_solution_id;
    
    void draw_rectangle(size_t const lx,
                        size_t const ly,
                        size_t const ux,
                        size_t const uy,
                        std::string const & label="");
};

} /* namespace Placer */

#endif /* PLOTTER_HPP */
