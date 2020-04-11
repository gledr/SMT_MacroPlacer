//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : terminal.hpp
//
// Date         : 22. December 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : SoC Terminal Component
//==================================================================
#ifndef TERMINAL_HPP
#define TERMINAL_HPP

#include <iostream>
#include <string>

#include <object.hpp>
#include <pin.hpp>
#include <encoding_utils.hpp>

namespace Placer {

/**
 * @class Terminal
 * @brief Soc Terminal Pin
 */
class Terminal: public virtual Object {
public:
    
    Terminal(std::string const & name,
             e_pin_direction const direction);

    Terminal(std::string const & name,
             size_t const pos_x,
             size_t const pos_y,
             e_pin_direction const direction);

    virtual ~Terminal();

    bool is_free();
    
    void set_direction(e_pin_direction const direction);
    e_pin_direction get_pin_direction() const;
    
    z3::expr& get_pin_pos_x();
    z3::expr& get_pin_pos_y();
    
    void add_solution_pin_pos_x(size_t const val);
    void add_solution_pin_pos_y(size_t const val);
    
    size_t get_solution_pin_pos_x(size_t const sol);
    size_t get_solution_pin_pos_y(size_t const sol);

    std::string get_name();
    std::string get_id();

    bool is_input();
    bool is_output();
    bool is_bidirectional();

    virtual void dump(std::ostream & stream = std::cout);

private:
    EncodingUtils* m_encode;
    std::string m_name;
    bool m_free;
    e_pin_direction m_direction;

    z3::expr m_pin_pos_x;
    z3::expr m_pin_pos_y;

    std::vector<size_t> m_solutions_x;
    std::vector<size_t> m_solutions_y;
};

}

#endif /* TERMINAL_HPP */
