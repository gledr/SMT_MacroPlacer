//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : supplementterminal.hpp
//
// Date         : 25.May 2020
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Supplement Terminal Instance
//==================================================================
#ifndef SUPPLEMENTTERMINAL_HPP
#define SUPPLEMENTTERMINAL_HPP

#include <string>
#include <iostream>

namespace Placer {

class SupplementTerminal {
public:
    SupplementTerminal(std::string const & pin_name);

    virtual ~SupplementTerminal();

    bool has_frequency();
    bool has_bitwidth();

    size_t get_frequency() const;
    size_t get_bitwidth() const;

    void set_frequency(size_t const frequency);
    void set_bitwidth(size_t const bitwidht);

    std::string get_name() const;

    std::ostream& operator<< (std::ostream & stream);

private:
    std::string m_name;
    size_t m_bitwidth;
    size_t m_frequency;
};

}
#endif /* SUPPLEMENTTERMINAL_HPP */
