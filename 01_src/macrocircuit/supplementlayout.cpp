//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : supplementlayout.cpp
//
// Date         : 14. May 2020
// Compiler     : gcc version 9.3.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Supplement Layout Instance
//==================================================================
#include "supplementlayout.hpp"

using namespace Placer;

bool debug = true;

SupplementLayout::SupplementLayout(size_t const lx,
                                   size_t const ly,
                                   size_t const ux,
                                   size_t const uy):
    m_ly(ly),
    m_lx(lx),
    m_uy(uy),
    m_ux(ux)
{
    debug && std::cout << "[Info]: New Supplement Layout (" 
                       << lx << ":" << ly << ") " 
                       << "(" << ux << ":" << uy << ")" << std::endl;
}

SupplementLayout::~SupplementLayout()
{
}

size_t SupplementLayout::get_lx() const
{
    return m_lx;
}

size_t SupplementLayout::get_ly() const
{
    return m_ly;
}

size_t SupplementLayout::get_ux() const
{
    return m_ux;
}

size_t SupplementLayout::get_uy() const
{
    return m_uy;
}
