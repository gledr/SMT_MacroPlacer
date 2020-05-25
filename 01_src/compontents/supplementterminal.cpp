//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : supplementpin.cpp
//
// Date         : 09. January 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Supplement Pin Instance
//==================================================================
#include "supplementterminal.hpp"

using namespace Placer;

static bool debug = true;

SupplementTerminal::SupplementTerminal(std::string const & pin_name):
    m_name(pin_name)
{
    debug && std::cout << "[Info]: New SupplementTerminal: " 
                       << pin_name << std::endl;
}

SupplementTerminal::~SupplementTerminal()
{
}

std::ostream& SupplementTerminal::operator<<(std::ostream& stream)
{
    stream<< "Pin: " << m_name << std::endl;
    if(this->has_bitwidth()){
        stream<< "Bitwidth: " << m_bitwidth << std::endl;
    }
    if(this->has_frequency()){
        stream << "Frequency: " << m_frequency << std::endl;
    }
    return stream;
}

size_t SupplementTerminal::get_bitwidth() const
{
    return m_bitwidth;
}

size_t SupplementTerminal::get_frequency() const
{
    return m_frequency;
}

std::string SupplementTerminal::get_name() const
{
    return m_name;
}

bool SupplementTerminal::has_bitwidth()
{
    return m_bitwidth != 0;
}

bool SupplementTerminal::has_frequency()
{
    return m_frequency != 0;
}

void SupplementTerminal::set_bitwidth(size_t const bitwidht)
{
    debug && std::cout << "[Info]: Setting bitwidth: " << bitwidht << std::endl;
    m_bitwidth = bitwidht;
}

void SupplementTerminal::set_frequency(size_t const frequency)
{
    debug && std::cout << "[Info]: Setting frequency: " << frequency << std::endl;
    m_frequency = frequency;
}
