//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : Pin.cpp
//
// Date         : 22. December 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : SoC Pin Component
//==================================================================
#include "pin.hpp"

using namespace Placer;
using namespace Placer::Utils;

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
    m_pin_pos_y(m_encode->get_value(y)),
    m_is_free(false),
    m_logger(Logger::getInstance())
{
    m_bitwidht = 0;
    m_frequency = 0;
    
    m_logger->add_fixed_pin(name, "", x, y);
}

/**
 * @brief Pin Name
 * 
 * @param pin_name PinName
 * @param macro_name Parent Macro Name
 * @param direction Pin Direction
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
    m_pin_pos_y(m_encode->get_constant(macro_name + pin_name + "_pos_y")),
    m_offset_x_percentage(0),
    m_offset_y_percentage(0),
    m_is_free(true),
    m_logger(Logger::getInstance())
{
    m_bitwidht = 0;
    m_frequency = 0;
    
    m_logger->add_free_pin(pin_name, macro_name);
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
 * @brief Check if bitwidth has every been assigned
 * 
 * @return bool
 */
bool Pin::has_bitwidth()
{
    return m_bitwidht != 0;
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
 * @brief Check if frequency has ever been assigned
 * 
 * @return bool
 */
bool Pin::has_frequency()
{
    return m_frequency != 0;
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
    stream << "Parent (" << m_macro_name << ")" << std::endl;
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
        notsupported_check("Invalid Pin Direction!");
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
 * @brief Get Pin Position as Numeral Value
 * 
 * @return size_t
 */
size_t Pin::get_pin_pos_x_numeral()
{
    assert (m_pin_pos_x.is_numeral());
    
    return m_pin_pos_x.get_numeral_uint();
}

/**
 * @brief Get Pin Position as Numeral Value
 * 
 * @return size_t
 */
size_t Pin::get_pin_pos_y_numeral()
{
    assert (m_pin_pos_y.is_numeral());
    
    return m_pin_pos_y.get_numeral_uint();
}

/**
 * @brief Add Solution for X Coordinate
 * 
 * @param val Coordinate
 */
void Pin::add_solution_pin_pos_x(size_t const val)
{
    m_logger->place_pin_x(m_pin_name, m_macro_name, val);

    m_solutions_x.push_back(val);
}

/**
 * @brief Add Solution for Y Coordinate
 * 
 * @param val Coordinate
 */
void Pin::add_solution_pin_pos_y(size_t const val)
{
    m_logger->place_pin_y(m_pin_name, m_macro_name, val);

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
 * @brief Set X Coordinate Offset to Center in Percentage
 * 
 * @param offset Offset
 */
void Pin::set_x_offset_percentage(int const offset)
{
    m_offset_x_percentage = offset;
}

/**
 * @brief Set Y Coordinate Offset to Center in Percentage
 * 
 * @param offset Offset
 */
void Pin::set_y_offset_percentage(int const offset)
{
    m_offset_y_percentage = offset;
}

/**
 * @brief Get X Offset from Center in Percentage
 * 
 * @return int
 */
int Pin::get_offset_x_percentage()
{
    return m_offset_x_percentage;
}

/**
 * @brief Get Y Offset from Center in Percentage
 * 
 * @return int
 */
int Pin::get_offset_y_percentage()
{
    return m_offset_y_percentage;
}

/**
 * @brief Check if X Offset is Defined
 * 
 * @return bool
 */
bool Pin::has_offset_x()
{
    return m_offset_x_percentage != 0;
}

/**
 * @brief Check if Y Offset is Defined
 * 
 * @return bool
 */
bool Pin::has_offset_y()
{
    return m_offset_y_percentage != 0;
}

/**
 * @brief Check if Pin if Placed or Free
 * 
 * @return bool
 */
bool Pin::is_free()
{
    return m_is_free;
}

/**
 * @brief Check if Pin Commands over certain solution
 *
 * @param solution_id p_solution_id:...
 * @return bool
 */
bool Pin::has_solution(size_t const solution_id)
{
    assert (m_solutions_x.size() == m_solutions_y.size());
    
    return m_solutions_x.size() > solution_id;
}
