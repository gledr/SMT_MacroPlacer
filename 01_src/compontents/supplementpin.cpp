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
#include "supplementpin.hpp"

using namespace Placer;

static bool debug = true;

SupplementPin::SupplementPin(std::string const & pin_name):
    m_pinname(pin_name)
{
    debug && std::cout << "[Info]: New SupplementPin: " 
                       << pin_name << std::endl;
}

SupplementPin::~SupplementPin()
{
}

std::ostream& SupplementPin::operator<<(std::ostream& stream)
{
    stream<< "Pin: " << m_pinname << std::endl;
    if(this->has_bitwidth()){
        stream<< "Bitwidth: " << m_bitwidth << std::endl;
    }
    if(this->has_frequency()){
        stream << "Frequency: " << m_frequency << std::endl;
    }
    return stream;
}

size_t SupplementPin::get_bitwidth() const
{
    return m_bitwidth;
}

size_t SupplementPin::get_frequency() const
{
    return m_frequency;
}

std::string SupplementPin::get_pinname() const
{
    return m_pinname;
}

bool SupplementPin::has_bitwidth()
{
    return m_bitwidth != 0;
}

bool SupplementPin::has_frequency()
{
    return m_frequency != 0;
}

void SupplementPin::set_bitwidth(size_t const bitwidht)
{
    debug && std::cout << "[Info]: Setting bitwidth: " << bitwidht << std::endl;
    m_bitwidth = bitwidht;
}

void SupplementPin::set_frequency(size_t const frequency)
{
    debug && std::cout << "[Info]: Setting frequency: " << frequency << std::endl;
    m_frequency = frequency;
}
