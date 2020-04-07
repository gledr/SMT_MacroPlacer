//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : Terminal.cpp
//
// Date         : 22. December 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : SoC Terminal Component
//==================================================================
#include "pin.hpp"

using namespace Placer;

/**
 * @brief ...
 * 
 * @param name p_name:...
 * @param direction p_direction:...
 * @param z3_ctx p_z3_ctx:...
 * @param x p_x:...
 * @param y p_y:...
 */
Pin::Pin(std::string const & name,
         e_pin_direction const direction,
         z3::context* z3_ctx,
         size_t const & x,
         size_t const & y):
    Object(),
    m_z3_ctx(z3_ctx),
    m_pin_name(name),
    m_direction(direction),
    m_pin_pos_x(z3_ctx->int_val(x)),
    m_pin_pos_y(z3_ctx->int_val(y))
{
    assert(z3_ctx != nullptr);
    this->get_verbose() && std::cout << "[Info]: Adding Placed Pin " << name << std::endl;
}

/**
 * @brief ...
 * 
 * @param pin_name p_pin_name:...
 * @param macro_name p_macro_name:...
 * @param direction p_direction:...
 * @param z3_ctx p_z3_ctx:...
 */
Pin::Pin(std::string const & pin_name,
         std::string const & macro_name,
         e_pin_direction const direction,
         z3::context* z3_ctx):
    Object(),
    m_z3_ctx(z3_ctx),
    m_pin_name(pin_name),
    m_macro_name(macro_name),
    m_direction(direction),
    m_pin_pos_x(z3_ctx->int_const(std::string(macro_name + pin_name + "_pos_x").c_str())),
    m_pin_pos_y(z3_ctx->int_const(std::string(macro_name + pin_name + "_pos_y").c_str()))
{
    assert(z3_ctx != nullptr);
    this->get_verbose() && std::cout << "[Info]: Adding Free Pin " << macro_name << "_" << pin_name << std::endl;
}

/**
 * @brief ...
 */
Pin::~Pin()
{
}

/**
 * @brief ...
 * 
 * @return std::string
 */
std::string Pin::get_name()
{
    return m_pin_name;
}

/**
 * @brief ...
 * 
 * @param width p_width:...
 */
void Pin::set_bitwidth(size_t const width)
{
    m_bitwidht = width;
}

/**
 * @brief ...
 * 
 * @return size_t
 */
size_t Pin::get_bitwidth() const
{
    return m_bitwidht;
}

/**
 * @brief ...
 * 
 * @return std::string
 */
std::string Pin::get_macro_name()
{
    return m_macro_name;
}

/**
 * @brief ...
 * 
 * @param frequency p_frequency:...
 */
void Pin::set_frequency(size_t const frequency)
{
    m_frequency = frequency;
}

/**
 * @brief ...
 * 
 * @return size_t
 */
size_t Pin::get_frequency() const
{
    return m_frequency;
}

/**
 * @brief ...
 * 
 * @param stream p_stream:...
 */
void Pin::dump(std::ostream& stream)
{
    stream << std::string(30, '~') << std::endl;
    stream << "Pin (" << m_pin_name << ")" << std::endl;
    stream << "Direction (" << Pin::enum2string(m_direction) << ")" << std::endl;
    stream << std::string(30, '~') << std::endl;
}

/**
 * @brief ...
 * 
 * @param name p_name:...
 * @return Placer::e_pin_direction
 */
e_pin_direction Pin::string2enum(std::string const & name)
{
    if(name == "INPUT"){
        return eInput;
    } else if (name == "OUTPUT"){
        return eOutput;
    } else if (name == "INOUT"){
       return eBidirectional;
    } else {
        return eUnknown;
    }
}

/**
 * @brief ...
 * 
 * @param direction p_direction:...
 * @return std::string
 */
std::string Pin::enum2string(e_pin_direction const direction)
{
    if(direction == eInput){
        return "INPUT";
    } else if (direction == eOutput){
        return "OUTPUT";
    }  else if (direction == eBidirectional){
        return "BIDIRECTION";
    } else if (direction == eUnknown){
        return "UNKOWN";
    } else {
        assert(0);
    }
}

/**
 * @brief ...
 * 
 * @return std::string
 */
std::string Pin::get_id()
{
    return m_pin_name;
}

/**
 * @brief ...
 * 
 * @return bool
 */
bool Pin::is_input()
{
    return m_direction == eInput;
}

/**
 * @brief ...
 * 
 * @return bool
 */
bool Pin::is_output()
{
    return m_direction == eOutput;
}

/**
 * @brief ...
 * 
 * @return z3::expr&
 */
z3::expr& Pin::get_pin_pos_x()
{
    return m_pin_pos_x;
}

/**
 * @brief ...
 * 
 * @return z3::expr&
 */
z3::expr& Pin::get_pin_pos_y()
{
    return m_pin_pos_y;
}

/**
 * @brief ...
 * 
 * @param val p_val:...
 */
void Pin::add_solution_pin_pos_x(size_t const val)
{
    this->get_verbose() && std::cout << "[Debug]: Set Solution for x: " << val << std::endl;

    m_solutions_x.push_back(val);
}

/**
 * @brief ...
 * 
 * @param val p_val:...
 */
void Pin::add_solution_pin_pos_y(size_t const val)
{
    this->get_verbose() && std::cout << "[Debug]: Set Solution for y: " << val << std::endl;

    m_solutions_y.push_back(val);
}

/**
 * @brief ...
 * 
 * @param sol p_sol:...
 * @return size_t
 */
size_t Pin::get_solution_pin_pos_x(size_t const  sol)
{
    return m_solutions_x[sol];
}

/**
 * @brief ...
 * 
 * @param sol p_sol:...
 * @return size_t
 */
size_t Pin::get_solution_pin_pos_y(size_t const sol)
{
    return m_solutions_y[sol];
}

/**
 * @brief ...
 * 
 * @param offset p_offset:...
 */
void Pin::set_x_offset(size_t const offset)
{
    m_offset_x = offset;
}

/**
 * @brief ...
 * 
 * @param offset p_offset:...
 */
void Pin::set_y_offset(size_t const offset)
{
    m_offset_y = offset;
}
