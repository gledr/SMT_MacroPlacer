//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : component.cpp
//
// Date         : 14. March 2020
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Super Class for Macros/Cells and Partitions
//==================================================================
#include "component.hpp"

using namespace Placer;
using namespace Placer::Utils;

/**
 * @brief Constructor
 */
Component::Component():
    m_encode(new EncodingUtils()),
    m_lx(m_encode->get_value(0)),
    m_ly(m_encode->get_value(0)),
    m_orientation(m_encode->get_value(0)),
    m_width(m_encode->get_value(0)),
    m_height(m_encode->get_value(0))
{
}

/**
 * @brief Constructor
 */
Component::~Component()
{
    delete m_encode; m_encode = nullptr;
}

/**
 * @brief Get lx coordinate Clauses
 * 
 * @return z3::expr&
 */
z3::expr& Component::get_lx()
{
    return this->m_lx;
}

/**
 * @brief Get ly coordinate Clauses
 * 
 * @return z3::expr&
 */
z3::expr& Component::get_ly()
{
    return this->m_ly;
}

/**
 * @brief Get Orientation Clauses
 * 
 * @return z3::expr&
 */
z3::expr& Component::get_orientation()
{
    return this->m_orientation;
}

/**
 * @brief Get Component height (vertical)
 */
z3::expr& Component::get_height()
{
    return m_height;
}

/**
 * @brief Get Component Widht (horizontal)
 *
 * @return z3::expr&
 */
z3::expr& Component::get_width()
{
    return m_width;
}

/**
 * @brief Get Height
 * 
 * @return size_t
 */
size_t Component::get_height_numeral()
{
    assertion_check (m_height.is_numeral());

    return m_height.get_numeral_uint();
}

/**
 * @brief Get Width
 * 
 * @return size_t
 */
size_t Component::get_width_numeral()
{
    assertion_check(m_width.is_numeral());

    return m_width.get_numeral_uint();
}

/**
 * @brief Get Component Name
 * 
 * @return std::string
 */
std::string Component::get_name()
{
    return m_name;
}

/**
 * @brief Get Component ID
 *
 * @return std::string
 */
std::string Component::get_id()
{
    return m_id;
}

/**
 * @brief Add Component lx coordinate for next solution
 * 
 * @param lx Lx coordinate
 */
void Component::add_solution_lx(size_t const lx)
{
    m_sol_lx.push_back(lx);
}

/**
 * @brief Add Component ly coordinate for next solution
 * 
 * @param ly Ly coordinate
 */
void Component::add_solution_ly(size_t const ly)
{
    m_sol_ly.push_back(ly);
}

/**
 * @brief Add Component orientation for next solution
 * 
 * @param orientation Orientation of next solution
 */
void Component::add_solution_orientation(eOrientation const orientation)
{
    m_sol_orientation.push_back(orientation);
}

/**
 * @brief Get lx coordinate depdending on component rotation
 * 
 * @param orientation Orientation of the Component
 * @return z3::expr
 */
z3::expr Component::get_lx(eOrientation const orientation)
{
    if (orientation == eNorth){
        return m_lx;
    } else if (orientation == eWest){
        return  m_encode->sub(m_lx, m_height);
    } else if (orientation == eSouth){
        return m_encode->sub(m_lx, m_width);
    } else if (orientation == eEast){
        return m_lx;
    } else {
        notimplemented_check();
    }
}

std::stringstream Component::_get_lx(eOrientation const orientation)
{
    std::stringstream ret_val;
    
    if (orientation == eNorth){
        ret_val << m_lx;
    } else if (orientation == eWest){
        ret_val << "(" << m_lx << " - " << m_height << ")";
    } else if (orientation == eSouth){
       ret_val << "(" << m_lx << " -" << m_width << ")";
    } else if (orientation == eEast){
       ret_val << m_lx;
    } else {
        notimplemented_check();
    }
    
    return ret_val;
}
/**
 * @brief Get lxy coordinate depdending on component rotation
 * 
 * @param orientation Orientation of the Component
 * @return z3::expr
 */
z3::expr Component::get_ly(eOrientation const orientation)
{
    if (orientation == eNorth){
        return m_ly;
    } else if (orientation == eWest){
        return m_ly;
    } else if (orientation == eSouth){
        return m_encode->sub(m_ly, m_height);
    } else if (orientation == eEast){
        return m_encode->sub(m_ly, m_width);
    } else {
        notimplemented_check();
    }
}

std::stringstream Component::_get_ly(eOrientation const orientation)
{
    std::stringstream ret_val;

    if (orientation == eNorth){
        ret_val << m_ly;
    } else if (orientation == eWest){
        ret_val << m_ly;
    } else if (orientation == eSouth){
        ret_val << "(" << m_ly << "-" << m_height << ")";
    } else if (orientation == eEast){
        ret_val << "(" << m_ly << "-" << m_width << ")";
    } else {
        notimplemented_check();
    }
    return ret_val;
}

/**
 * @brief Get ux coordinate depdending on component rotation
 * 
 * @param orientation Orientation of the Component
 * @return z3::expr
 */
z3::expr Component::get_ux(eOrientation const orientation)
{
    if (orientation == eNorth){
        return m_encode->add(m_lx, m_width);
    } else if (orientation == eWest){
        return m_lx;
    } else if (orientation == eSouth){
        return m_lx;
    } else if (orientation == eEast){
        return m_encode->add(m_lx, m_height);
    } else {
        notimplemented_check();
    }
}

std::stringstream Component::_get_ux(eOrientation const orientation)
{
    std::stringstream ret_val;
    
    if (orientation == eNorth){
        ret_val << "(" << m_lx << "+" << m_width << ")";
    } else if (orientation == eWest){
        ret_val << m_lx;
    } else if (orientation == eSouth){
        ret_val << m_lx;
    } else if (orientation == eEast){
        ret_val << "(" << m_lx << "+" << m_height << ")";
    } else {
        notimplemented_check();
    }
    
    return ret_val;
}

/**
 * @brief Get uy coordinate depdending on component rotation
 * 
 * @param orientation Orientation of the Component
 * @return z3::expr
 */
z3::expr Component::get_uy(eOrientation const orientation)
{
    if (orientation == eNorth){
        return m_encode->add(m_ly, m_height);
    } else if (orientation == eWest){
        return m_encode->add(m_ly, m_width);
    } else if (orientation == eSouth){
        return m_ly;
    } else if (orientation == eEast){
        return m_ly;
    } else {
        notimplemented_check();
    }
}

std::stringstream Component::_get_uy(eOrientation const orientation)
{
    std::stringstream ret_val;

    if (orientation == eNorth){
        ret_val << "(" << m_ly << "+" << m_height << ")";
    } else if (orientation == eWest){
        ret_val << "(" << m_ly << "+" << m_width << ")";
    } else if (orientation == eSouth){
        ret_val << m_ly;
    } else if (orientation == eEast){
        ret_val << m_ly;
    } else {
        notimplemented_check();
    }
    return ret_val;
}

/**
 * @brief Get Solution for LY
 * 
 * @param id Solution ID
 * @return size_t
 */
size_t Component::get_solution_ly(size_t const id)
{
    assertion_check(id < m_sol_ly.size());

    return  m_sol_ly[id];
}

/**
 * @brief Get Solution for LX
 * 
 * @param id Solution ID
 * @return size_t
 */
size_t Component::get_solution_lx(size_t const id)
{
    assertion_check (id < m_sol_lx.size());

    return m_sol_lx[id];
}

/**
 * @brief Get Solution for Orientation
 * 
 * @param id Solution ID
 * @return Placer::eOrientation
 */
eOrientation Component::get_solution_orientation(size_t const id)
{
    assertion_check (id < m_sol_orientation.size());

    return m_sol_orientation[id];
}

/**
 * @brief Check if Solution Exists
 * 
 * @param id Solution ID to Check
 * @return bool
 */
bool Component::has_solution(size_t const id)
{
    assertion_check (m_sol_lx.size() == m_sol_ly.size());
    assertion_check (m_sol_ly.size() == m_sol_orientation.size());

    return (m_sol_lx.size()) > id;
}

/**
 * @brief Check Macro Position North Clause
 * 
 * @return z3::expr
 */
z3::expr Component::is_N()
{
    return m_orientation == m_encode->get_value(eNorth);
}

/**
 * @brief Check West Position North Clause
 * 
 * @return z3::expr
 */
z3::expr Component::is_W()
{
    return m_orientation == m_encode->get_value(eWest);
}

/**
 * @brief Check Macro Position South Clause
 * 
 * @return z3::expr
 */
z3::expr Component::is_S()
{
    return m_orientation == m_encode->get_value(eSouth);
}

/**
 * @brief Check Macro Position East Clause
 * 
 * @return z3::expr
 */
z3::expr Component::is_E()
{
    return m_orientation == m_encode->get_value(eEast);
}

/**
 * @brief 
 * 
 * @return std::string
 */
std::string Component::_is_N()
{
    std::stringstream builder;
    builder << m_orientation << " == " << std::to_string(eNorth);
    
    return builder.str();
}

/**
 * @brief 
 * 
 * @return std::string
 */
std::string Component::_is_W()
{
    std::stringstream builder;
    builder << m_orientation << " == " << std::to_string(eWest);
    
    return builder.str();
}

/**
 * @brief 
 * 
 * @return std::string
 */
std::string Component::_is_S()
{
    assert (0);
}

/**
 * @brief 
 * 
 * @return std::string
 */
std::string Component::_is_E()
{
    assert (0);
}

/**
 * @brief 
 * 
 * @return size_t
 */
size_t Component::get_lx_numeral()
{
    return m_lx.get_numeral_uint();
}

/**
 * @brief 
 * 
 * @return size_t
 */
size_t Component::get_ly_numeral()
{
    return m_ly.get_numeral_uint();
}
