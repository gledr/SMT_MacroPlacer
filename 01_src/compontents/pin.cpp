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
 * @brief Constructor
 * 
 * @param name Pinname
 * @param direction Pin Direction
 * @param x Pin Location X
 * @param y Pin Location Y
 */
Pin::Pin(std::string const & name,
         e_pin_direction const direction,
         size_t const & x,
         size_t const & y):
    Object(),
    m_encode(new EncodingUtils()),
    m_pin_name(name),
    m_direction(direction),
    m_pin_pos_x(m_encode->get_value(x)),
    m_pin_pos_y(m_encode->get_value(y))
{
    this->get_verbose() && std::cout << "[Info]: Adding Placed Pin " << name << std::endl;
}

/**
 * @brief Pin Name
 * 
 * @param pin_name p_pin_name:...
 * @param macro_name p_macro_name:...
 * @param direction p_direction:...
 */
Pin::Pin(std::string const & pin_name,
         std::string const & macro_name,
         e_pin_direction const direction):
    Object(),
    m_encode(new EncodingUtils()),
    m_pin_name(pin_name),
    m_macro_name(macro_name),
    m_direction(direction),
    m_pin_pos_x(m_encode->get_constant(macro_name + pin_name + "_pos_x")),
    m_pin_pos_y(m_encode->get_constant(macro_name + pin_name + "_pos_y"))
{
    this->get_verbose() && std::cout << "[Info]: Adding Free Pin " << macro_name << "_" << pin_name << std::endl;
}

/**
 * @brief Destructor
 */
Pin::~Pin()
{
    delete m_encode; m_encode = nullptr;
}

/**
 * @brief Get Pinname
 * 
 * @return std::string
 */
std::string Pin::get_name()
{
    return m_pin_name;
}

/**
 * @brief Set Bitwidth of Pin
 * 
 * @param width Bitwidth
 */
void Pin::set_bitwidth(size_t const width)
{
    m_bitwidht = width;
}

/**
 * @brief Get Bitwidth of Pin
 * 
 * @return size_t
 */
size_t Pin::get_bitwidth() const
{
    return m_bitwidht;
}

/**
 * @brief Get Macro name Pin belongs to
 * 
 * @return std::string
 */
std::string Pin::get_macro_name()
{
    return m_macro_name;
}

/**
 * @brief Set Pin frequency
 * 
 * @param frequency Frequency
 */
void Pin::set_frequency(size_t const frequency)
{
    m_frequency = frequency;
}

/**
 * @brief Get Pin Frequency
 * 
 * @return size_t
 */
size_t Pin::get_frequency() const
{
    return m_frequency;
}

/**
 * @brief Dump Pin Information to given stream
 * 
 * @param stream Stream to dump to
 */
void Pin::dump(std::ostream& stream)
{
    stream << std::string(30, '~') << std::endl;
    stream << "Pin (" << m_pin_name << ")" << std::endl;
    stream << "Direction (" << Pin::enum2string(m_direction) << ")" << std::endl;
    stream << std::string(30, '~') << std::endl;
}

/**
 * @brief Translate given name to enum
 * 
 * @param name String to translate
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
 * @brief Translate given enum to string
 * 
 * @param direction Enum to translate
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
 * @brief Get Pin ID
 * 
 * @return std::string
 */
std::string Pin::get_id()
{
    return m_pin_name;
}

/**
 * @brief Check if Pin is Input
 * 
 * @return bool
 */
bool Pin::is_input()
{
    return m_direction == eInput;
}

/**
 * @brief Check if Pin is Output
 * 
 * @return bool
 */
bool Pin::is_output()
{
    return m_direction == eOutput;
}

/**
 * @brief Get Pin Position X Coordinate
 * 
 * @return z3::expr&
 */
z3::expr& Pin::get_pin_pos_x()
{
    return m_pin_pos_x;
}

/**
 * @brief Get Pin Position Y Coordinate
 * 
 * @return z3::expr&
 */
z3::expr& Pin::get_pin_pos_y()
{
    return m_pin_pos_y;
}

/**
 * @brief Add Solution for X Coordinate
 * 
 * @param val Coordinate
 */
void Pin::add_solution_pin_pos_x(size_t const val)
{
    this->get_verbose() && std::cout << "[Debug]: Set Solution for x: " << val << std::endl;

    m_solutions_x.push_back(val);
}

/**
 * @brief Add Solution for Y Coordinate
 * 
 * @param val Coordinate
 */
void Pin::add_solution_pin_pos_y(size_t const val)
{
    this->get_verbose() && std::cout << "[Debug]: Set Solution for y: " << val << std::endl;

    m_solutions_y.push_back(val);
}

/**
 * @brief Get Solution for X Coordinate
 * 
 * @param sol Solution ID
 * @return size_t
 */
size_t Pin::get_solution_pin_pos_x(size_t const  sol)
{
    return m_solutions_x[sol];
}

/**
 * @brief Get Solution for Y Coordinate
 * 
 * @param sol Solution ID
 * @return size_t
 */
size_t Pin::get_solution_pin_pos_y(size_t const sol)
{
    return m_solutions_y[sol];
}

/**
 * @brief Set X Coordinate Offset to Center
 * 
 * @param offset Offset
 */
void Pin::set_x_offset(size_t const offset)
{
    m_offset_x = offset;
}

/**
 * @brief Set Y Coordinate Offset to Center
 * 
 * @param offset p_offset:...
 */
void Pin::set_y_offset(size_t const offset)
{
    m_offset_y = offset;
}
