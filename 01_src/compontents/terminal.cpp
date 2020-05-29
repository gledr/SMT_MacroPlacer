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


/**
 * @brief Constructor for Free Placeable Terminal
 * 
 * @param name Terminal Name
 * @param direction Terminal Direction
 */
Terminal::Terminal(std::string const & name,
                  e_pin_direction const direction):
    Object(),
    m_logger(Utils::Logger::getInstance()),
    m_encode(new EncodingUtils()),
    m_name(name),
    m_free(true),
    m_direction(direction),
    m_pos_x(m_encode->get_constant("terminal_" + name + "_x")),
    m_pos_y(m_encode->get_constant("terminal_" + name + "_y")),
    m_key(m_key_counter++)
{
    m_free = true;
    m_bitwidth = 0;
    m_frequency = 0;
    m_logger->add_free_terminal(name);
}

/**
 * @brief Constructor for Fixed Placed Terminal
 * 
 * @param name Terminal Name
 * @param pos_x Terminal Position X
 * @param pos_y Terminal Position Y
 * @param direction Terminal Direction
 */
Terminal::Terminal(std::string const & name,
                   size_t const pos_x,
                   size_t const pos_y,
                   e_pin_direction const direction):
    Object(),
    m_logger(Utils::Logger::getInstance()),
    m_encode(new EncodingUtils()),
    m_name(name),
    m_free(false),
    m_direction(direction),
    m_pos_x(m_encode->get_value(pos_x)),
    m_pos_y(m_encode->get_value(pos_y)),
    m_key(m_key_counter++)
{
    m_free = false;
    m_bitwidth = 0;
    m_frequency = 0;
    m_logger->add_fixed_terminal(name, pos_x, pos_y);
}

/**
 * @brief Destructor
 */
Terminal::~Terminal()
{
    delete m_encode; m_encode = nullptr;
    m_logger = nullptr;
}

/**
 * @brief Get Terminal Name
 * 
 * @return std::string
 */
std::string Terminal::get_name()
{
    return m_name;
}

/**
 * @brief Get Unique ID of Terminal
 * 
 * @return std::string
 */
std::string Terminal::get_id()
{
    return m_name;
}

/**
 * @brief Get Terminal Key
 * 
 * @return size_t
 */
size_t Terminal::get_key()
{
    return m_key;
}

/**
 * @brief Get X Position Clause
 * 
 * @return z3::expr&
 */
z3::expr& Terminal::get_pos_x()
{
    return m_pos_x;
}

/**
 * @brief Get Y Position Clause
 * 
 * @return z3::expr&
 */
z3::expr& Terminal::get_pos_y()
{
    return m_pos_y;
}

/**
 * @brief Add Solution for the X Value
 * 
 * @param val X-Position
 */
void Terminal::add_solution_pos_x(size_t const val)
{
    m_solutions_x.push_back(val);
}

/**
 * @brief Add Solution for the Y Value
 * 
 * @param val Y-Position
 */
void Terminal::add_solution_pos_y(size_t const val)
{
    m_solutions_y.push_back(val);
}

/**
 * @brief Get Value for X-Position for a given Solution
 * 
 * @param sol Solution ID
 * @return size_t
 */
size_t Terminal::get_solution_pos_x(size_t const  sol)
{
    return m_solutions_x[sol];
}

/**
 * @brief Get Value for Y-Position for a given Solution
 * 
 * @param sol Solution ID
 * @return size_t
 */
size_t Terminal::get_solution_pos_y(size_t const sol)
{
    return m_solutions_y[sol];
}

/**
 * @brief Check if Terminal is Free
 * 
 * Free in this sense means it has not fixed location
 * 
 * @return bool
 */
bool Terminal::is_free()
{
    return m_free;
}

/**
 * @brief Check if Terminal is Input
 * 
 * @return bool
 */
bool Terminal::is_input()
{
    return m_direction == eInput;
}

/**
 * @brief Check if Terminal is Output
 * 
 * @return bool
 */
bool Terminal::is_output()
{
    return m_direction == eOutput;
}

/**
 * @brief Check if Terminal is Bidirectional
 * 
 * @return bool
 */
bool Terminal::is_bidirectional()
{
    return m_direction == eBidirectional;
}

/**
 * @brief Dump Information to a given stream
 * 
 * @param stream Stream to dump to
 */
void Terminal::dump(std::ostream& stream)
{
    stream << std::string(30, '*') << std::endl;
    stream << "Terminal (" << m_name << ")" << std::endl;
    stream << "Direction (" << Pin::enum2string(m_direction) << ")" << std::endl;
    stream << std::string(30, '*') << std::endl;
}

/**
 * @brief Get Direction
 * 
 * @return Placer::e_pin_direction
 */
e_pin_direction Terminal::get_direction() const
{
    return m_direction;
}

/**
 * @brief Set Direction
 * 
 * @param direction Direction of the Terminal
 */
void Terminal::set_direction(e_pin_direction const direction)
{
    m_direction = direction;
}

/**
 * @brief Check if Terminal has given solution
 * 
 * @param solution Solution to Check
 * @return bool
 */
bool Terminal::has_solution(size_t const solution)
{
    assertion_check (m_solutions_x.size() == m_solutions_y.size());

    return solution < m_solutions_x.size();
}

/**
 * @brief Get Terminal BitWidth
 * 
 * @return size_t
 */
size_t Terminal::get_bitwidth() const
{
    return m_bitwidth;
}

/**
 * @brief Get Terminal Frequency
 * 
 * @return size_t
 */
size_t Terminal::get_frequency() const
{
    return m_frequency;
}

/**
 * @brief Check if Terminal has BitWidth
 * 
 * @return bool
 */
bool Terminal::has_bitwidth()
{
    return m_bitwidth != 0;
}

/**
 * @brief Check if Terminal has Frequency
 */
bool Terminal::has_frequency()
{
    return m_frequency != 0;
}

/**
 * @brief Set Terminal BitWidth
 * 
 * @param width: BitWidth
 */
void Terminal::set_bitwidth(size_t const width)
{
    m_bitwidth = width;
}

/**
 * @brief Set Terminal Frequency
 * 
 * @param frequency Frequency
 */
void Terminal::set_frequency(size_t const frequency)
{
    m_frequency = frequency;
}
