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
#ifndef SUPPLEMENTLAYOUT_HPP
#define SUPPLEMENTLAYOUT_HPP

#include <string>
#include <iostream>

namespace Placer {

class SupplementLayout {
public:
    SupplementLayout(size_t const lx,
                     size_t const ly,
                     size_t const ux,
                     size_t const uy);

    virtual ~SupplementLayout();
    
    size_t get_lx() const;
    size_t get_ly() const;
    size_t get_ux() const;
    size_t get_uy() const;

private:
    size_t m_ly;
    size_t m_lx;
    size_t m_uy;
    size_t m_ux;
};

} /* namespace Placer */

#endif /* SUPPLEMENTLAYOUT_HPP */
