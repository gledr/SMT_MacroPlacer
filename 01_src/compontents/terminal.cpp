//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : terminal.cpp
//
// Date         : 22. December 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : SoC Terminal Component
//==================================================================
#include "terminal.hpp"

using namespace Placer;


static bool debug = false;

/**
 * @brief Constructor for Free Placeable Terminal
 * 
 * @param name Terminal Name
 * @param z3_ctx Z3 Context
 * @param direction Terminal Direction
 */
Terminal::Terminal(std::string const & name,
                  z3::context* z3_ctx,
                  e_pin_direction const direction):
    m_name(name),
    m_z3_ctx(z3_ctx),
    m_direction(direction),
    m_pin_pos_x(z3_ctx->int_const(std::string("terminal_" + name + "_x").c_str())),
    m_pin_pos_y(z3_ctx->int_const(std::string("terminal_" + name + "_y").c_str())),
    m_free(true)
{
    assert(z3_ctx != nullptr);
    m_free = true;
    debug && std::cout << "[Info]: Added Free Terminal " << name << std::endl;
}

/**
 * @brief Constructor for Fixed Placed Terminal
 * 
 * @param name Terminal Name
 * @param pos_x Terminal Position X
 * @param pos_y Terminal Position Y
 * @param z3_ctx Z3 Context
 * @param direction Terminal Direction
 */
Terminal::Terminal(std::string const & name,
                           size_t const pos_x,
                           size_t const pos_y,
                           z3::context* z3_ctx,
                           e_pin_direction const direction):
    m_name(name),
    m_z3_ctx(z3_ctx),
    m_direction(direction),
    m_pin_pos_x(z3_ctx->int_val(pos_x)),
    m_pin_pos_y(z3_ctx->int_val(pos_y)),
    m_free(false)
{
    assert(z3_ctx != nullptr);
    m_free = false;
    debug && std::cout << "[Info]: Added Fixed Terminal " 
                << name << "(" << pos_x << ":" 
                << pos_y << ")" << std::endl;
}

Terminal::~Terminal()
{
}

std::string Terminal::get_name()
{
    return m_name;
}

std::string Terminal::get_id()
{
    return m_name;
}

z3::expr& Terminal::get_pin_pos_x()
{
    return m_pin_pos_x;
}

z3::expr& Terminal::get_pin_pos_y()
{
    return m_pin_pos_y;
}

void Terminal::add_solution_pin_pos_x(size_t const val)
{
    debug && std::cout << "[Debug]: Set Solution for x: " << val << std::endl;

    m_solutions_x.push_back(val);
}

void Terminal::add_solution_pin_pos_y(size_t const val)
{
    debug && std::cout << "[Debug]: Set Solution for y: " << val << std::endl;

    m_solutions_y.push_back(val);
}

size_t Terminal::get_solution_pin_pos_x(size_t const  sol)
{
    return m_solutions_x[sol];
}

size_t Terminal::get_solution_pin_pos_y(size_t const sol)
{
    return m_solutions_y[sol];
}

bool Terminal::is_free()
{
    return m_free;
}

bool Terminal::is_input()
{
    return m_direction == eInput;
}

bool Terminal::is_output()
{
    return m_direction == eOutput;
}

bool Terminal::is_bidirectional()
{
    return m_direction == eBidirectional;
}

void Terminal::dump(std::ostream& stream)
{
    stream << std::string(30, '*') << std::endl;
    stream << "Terminal (" << m_name << ")" << std::endl;
    stream << "Direction (" << Pin::enum2string(m_direction) << ")" << std::endl;
    stream << std::string(30, '*') << std::endl;
}

e_pin_direction Terminal::get_pin_direction() const
{
    return m_direction;
}

void Terminal::set_direction(e_pin_direction const direction)
{
    m_direction = direction;
}
