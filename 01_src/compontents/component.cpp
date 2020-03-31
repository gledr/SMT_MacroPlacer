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
Component::Component(z3::context* z3_ctx):
    m_z3_ctx(z3_ctx),
    m_lx(z3_ctx->int_val(0)),
    m_ly(z3_ctx->int_val(0)),
    m_orientation(z3_ctx->int_val(0)),
    m_width(z3_ctx->int_val(0)),
    m_height(z3_ctx->int_val(0))
{
    assert (z3_ctx != nullptr);
}

/**
 * @brief Constructor
 */
Component::~Component()
{
}

/**
 * @brief .
 * 
 * @return z3::expr&
 */
z3::expr& Component::get_lx()
{
    return this->m_lx;
}

/**
 * @brief 
 * 
 * @return z3::expr&
 */
z3::expr& Component::get_ly()
{
    return this->m_ly;
}

/**
 * @brief 
 * 
 * @return std::string
 */
z3::expr& Component::get_orientation()
{
    return this->m_orientation;
}

/**
 * @brief 
 * 
 * @param ly 
 */
z3::expr& Component::get_height()
{
    return m_height;
}

/**
 * @brief 
 *
 * @return z3::expr&
 */
z3::expr& Component::get_width()
{
    return m_width;
}


/**
 * @brief 
 * 
 * @return std::string
 */
std::string Component::get_name()
{
    return m_name;
}

/**
 * @brief 
 *
 * @return std::string
 */
std::string Component::get_id()
{
    return m_id;
}

/**
 * @brief 
 * 
 * @param lx
 */
void Component::add_solution_lx(size_t const lx)
{
    m_sol_lx.push_back(lx);
}

/**
 * @brief 
 * 
 * @param ly 
 */
void Component::add_solution_ly(size_t const ly)
{
    m_sol_ly.push_back(ly);
}

/**
 * @brief 
 * 
 * @param orientation 
 */
void Component::add_solution_orientation(eOrientation const orientation)
{
    m_sol_orientation.push_back(orientation);
}
