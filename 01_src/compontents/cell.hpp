//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : cell.hpp
//
// Date         : 22. December 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Standard Cell Component
//==================================================================
#ifndef CELL_HPP
#define CELL_HPP

#include <string>
#include <vector>
#include <z3++.h>

#include <component.hpp>
#include <pin.hpp>

namespace Placer {
    
class Cell: public virtual Component{
public:
   
    Cell(z3::context* z3_ctx,
        std::string const & name,
        std::string const & id);

    virtual ~Cell();
    
    virtual void dump(std::ostream & stream = std::cout);

    void add_pin(Pin* _pin);
    
    virtual size_t get_area();
    
private: 

    std::vector<Pin*> m_pins;
};

}

#endif /* CELL_HPP */
