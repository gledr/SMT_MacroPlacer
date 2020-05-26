//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : supplementpin.hpp
//
// Date         : 09. January 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Supplement Pin Instance
//==================================================================
#ifndef SUPPLEMENTPIN_HPP
#define SUPPLEMENTPIN_HPP

#include <string>
#include <iostream>

namespace Placer {

class SupplementPin {
public:
    SupplementPin(std::string const & pin_name);

    virtual ~SupplementPin();
    
    bool has_frequency();
    bool has_bitwidth();
    
    size_t get_frequency() const;
    size_t get_bitwidth() const;
    
    void set_frequency(size_t const frequency);
    void set_bitwidth(size_t const bitwidht);
    
    std::string get_pinname() const;
    
    std::ostream& operator<< (std::ostream & stream);
    
private:
    std::string m_pinname;
    size_t m_bitwidth;
    size_t m_frequency;
};

} /* namespace Placer */

#endif /* SUPPLEMENTPIN_HPP */
