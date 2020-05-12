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
        assert (0);
    }
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
        assert (0);
    }
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
        assert (0);
    }
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
        assert (0);
    }
}

/**
 * @brief 
 * 
 * @param id 
 * @return size_t
 */
size_t Component::get_solution_ly(size_t const id)
{
    assert (id < m_sol_ly.size());
    
    return  m_sol_ly[id];
}

/**
 * @brief 
 * 
 * @param id 
 * @return size_t
 */
size_t Component::get_solution_lx(size_t const id)
{
    assert (id < m_sol_lx.size());
    
    return m_sol_lx[id];
}

/**
 * @brief 
 * 
 * @param id 
 * @return size_t
 */
size_t Component::get_solution_orientation(size_t const id)
{
    assert (id < m_sol_orientation.size());
    
    return m_sol_orientation[id];
}
