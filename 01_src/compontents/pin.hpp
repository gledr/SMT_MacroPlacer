//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : Terminal.hpp
//
// Date         : 22. December 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : SoC Terminal Component
//==================================================================
#ifndef PIN_HPP
#define PIN_HPP

#include <string>
#include <iostream>
#include <vector>
#include <z3++.h>

#include <object.hpp>
#include <encoding_utils.hpp>
#include <exception.hpp>
#include <logger.hpp>

namespace Placer {

enum e_pin_direction {eInput,
                      eOutput,
                      eBidirectional,
                      eUnknown};

class Pin: public virtual Object {
public:

    Pin(std::string const & pin_name,
        e_pin_direction const direction,
        size_t const & x,
        size_t const & y);

    Pin(std::string const & pin_name,
        std::string const & macro_name,
        e_pin_direction const direction);

    virtual ~Pin();

    void dump(std::ostream & stream = std::cout);

    z3::expr& get_pin_pos_x();
    z3::expr& get_pin_pos_y();

    size_t get_pin_pos_x_numeral();
    size_t get_pin_pos_y_numeral();

    void set_x_offset_percentage(int const offset);
    void set_y_offset_percentage(int const offset);

    bool has_offset_x();
    bool has_offset_y();

    int get_offset_x_percentage();
    int get_offset_y_percentage();

    void add_solution_pin_pos_x(size_t const val);
    void add_solution_pin_pos_y(size_t const val);

    size_t get_solution_pin_pos_x(size_t const sol);
    size_t get_solution_pin_pos_y(size_t const sol);

    bool is_input();
    bool is_output();
    virtual bool is_free();
    bool has_solution(size_t const solution_id);

    std::string get_name();
    std::string get_id();
    std::string get_macro_name();

    void set_bitwidth(size_t const width);
    size_t get_bitwidth () const;
    bool has_bitwidth();

    void set_frequency(size_t const frequency);
    size_t get_frequency() const;
    bool has_frequency();

    static e_pin_direction string2enum(std::string const & name);
    static std::string enum2string(e_pin_direction const direction);

private:
    size_t m_bitwidht;
    size_t m_frequency;

    EncodingUtils* m_encode;
    Utils::Logger* m_logger;

    std::string m_pin_name;
    std::string m_macro_name;
    e_pin_direction m_direction;

    z3::expr m_pin_pos_x;
    z3::expr m_pin_pos_y;

    std::vector<size_t> m_solutions_x;
    std::vector<size_t> m_solutions_y;

    int m_offset_x_percentage;
    int m_offset_y_percentage;
    bool m_is_free;
};

} /* namespace Placer */

#endif /* PIN_HPP */
