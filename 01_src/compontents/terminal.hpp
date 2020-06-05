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
#include <algorithm>

#include <pin.hpp>
#include <object.hpp>
#include <logger.hpp>
#include <encoding_utils.hpp>

namespace Placer {
    
enum eTerminalType {
    ePowerTerminal,
    eSignalTerminal,
    eUnknownTerminal};

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
    e_pin_direction get_direction() const;

    z3::expr& get_pos_x();
    z3::expr& get_pos_y();

    size_t get_pox_x_numerical();
    size_t get_pos_y_numerical();

    void add_solution_pos_x(size_t const val);
    void add_solution_pos_y(size_t const val);

    size_t get_solution_pos_x(size_t const sol);
    size_t get_solution_pos_y(size_t const sol);

    std::string get_name();
    std::string get_id();
    size_t get_key();
    bool has_solution(size_t const solution);

    bool is_input();
    bool is_output();
    bool is_bidirectional();

    bool is_power_terminal();
    bool is_signal_terminal();

    void set_bitwidth(size_t const width);
    size_t get_bitwidth () const;
    bool has_bitwidth();

    void set_frequency(size_t const frequency);
    size_t get_frequency() const;
    bool has_frequency();

    virtual void dump(std::ostream & stream = std::cout);

private:
    Utils::Logger* m_logger;
    EncodingUtils* m_encode;
    std::string m_name;
    bool m_free;
    size_t m_key;
    e_pin_direction m_direction;
    eTerminalType m_terminal_type;

    size_t m_bitwidth;
    size_t m_frequency;

    z3::expr m_pos_x;
    z3::expr m_pos_y;

    std::vector<size_t> m_solutions_x;
    std::vector<size_t> m_solutions_y;

    void resolve_terminal_type();
    static std::vector<std::string> m_terminal_keywords;
};

} /* namespace Placer */

#endif /* TERMINAL_HPP */
