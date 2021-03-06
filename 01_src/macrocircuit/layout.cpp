//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : layout.cpp
//
// Date         : 16. January 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Die Layout
//==================================================================
#include "layout.hpp"

using namespace Placer;
using namespace Placer::Utils;

/**
 * @brief Constructor
 *  
 * @param z3_ctx Z3 Context
 */
Layout::Layout ():
    Object(),
    m_encode(new EncodingUtils()),
    m_free_lx(true),
    m_free_ly(true),
    m_free_ux(true),
    m_free_uy(true),
    m_lx(m_encode->get_constant("die_lx")),
    m_ly(m_encode->get_constant("die_ly")),
    m_ux(m_encode->get_constant("die_ux")),
    m_uy(m_encode->get_constant("die_uy")),
    m_units(m_encode->get_constant("die_units"))
{
    m_logger = Logger::getInstance();
}

/**
 * @brief Destructor
 */
Layout::~Layout ()
{
    delete m_encode; m_encode = nullptr;
}

/**
 * @brief Get minimum die prediction
 * 
 * @return double
 */
double Layout::get_min_die_prediction()
{
    return m_min_die_size;
}

/**
 * @brief Set minimum die prediction
 * 
 * @param die_size Value to set
 */
void Layout::set_min_die_predition(double const & die_size)
{
    m_min_die_size = die_size;
}

/**
 * @brief Get lx clauses
 * 
 * @return z3::expr&
 */
z3::expr& Layout::get_lx()
{
    return m_lx;
}

/**
 * @brief Get ux clauses
 * 
 * @return z3::expr&
 */
z3::expr& Layout::get_ux()
{
    return m_ux;
}

/**
 * @brief Get ly clauses
 * 
 * @return z3::expr&
 */
z3::expr& Layout::get_ly()
{
    return m_ly;
}

/**
 * @brief Get uy clauses
 * 
 * @return z3::expr&
 */
z3::expr& Layout::get_uy()
{
    return m_uy;
}

/**
 * @brief Free ux variable
 */
void Layout::free_ux ()
{
    m_ux = m_encode->get_constant("die_ux");
    m_free_ux = true;
}

/**
 * @brief Free uy variable
 */
void Layout::free_uy ()
{
    m_uy = m_encode->get_constant("die_uy");
    m_free_uy = true;
}

/**
 * @brief Free lx variable
 */
void Layout::free_lx ()
{
    m_lx =m_encode->get_constant("die_lx");
    m_free_lx = true;
}

/**
 * @brief Free ly variable
 */
void Layout::free_ly ()
{
    m_ly = m_encode->get_constant("die_ly");
    m_free_ly = true;
}

/**
 * @brief Set value for lx variable
 * 
 * @param val Value for lx
 */
void Layout::set_lx(size_t const val)
{
    m_logger->set_die_lx(val);
    m_free_lx = false;
    m_lx = m_encode->get_value(val);
}

/**
 * @brief Set value for ly variable
 * 
 * @param val Value for ly
 */
void Layout::set_ly(size_t const val)
{
    m_logger->set_die_ly(val);
    m_free_ly = false;
    m_ly = m_encode->get_value(val);
}

/**
 * @brief Set value for ux variable
 * 
 * @param val Value for ux
 */
void Layout::set_ux(size_t const val)
{
    m_logger->set_die_ux(val);
    m_free_ux = false;
    m_ux = m_encode->get_value(val);
}

/**
 * @brief Set value for uy variable
 * 
 * @param val Value for uy
 */
void Layout::set_uy(size_t const val)
{
    m_logger->set_die_uy(val);
    m_free_uy = false;
    m_uy = m_encode->get_value(val);
}

/**
 * @brief Check if lx is free
 *
 * @return bool
 */
bool Layout::is_free_lx()
{
    return m_free_lx;
}

/**
 * @brief Check if ly is free
 * 
 * @return bool
 */
bool Layout::is_free_ly()
{
    return m_free_ly;
}

/**
 * @brief Check if uy is free
 * 
 * @return bool
 */
bool Layout::is_free_uy()
{
    return m_free_uy;
}

/**
 * @brief Check if ux is free
 * 
 * @return bool
 */
bool Layout::is_free_ux()
{
    return m_free_ux;
}

/**
 * @brief Get value of particular solution for ux
 * 
 * @param idx Solution id
 * @return size_t
 */
size_t Layout::get_solution_ux(size_t const idx)
{
    return m_solutions_ux[idx];
}

/**
 * @brief Get value of particular solution for uy
 * 
 * @param idx Solution id
 * @return size_t
 */
size_t Layout::get_solution_uy(size_t const idx)
{
    return m_solutions_uy[idx];
}

/**
 * @brief Add Solution for ux
 * 
 * @param val Solution Value
 * @return size_t
 */
void Layout::set_solution_ux(size_t const val)
{
    m_logger->add_solution_die_ux(val);
    m_solutions_ux.push_back(val);
}

/**
 * @brief Add Solution for uy
 * 
 * @param val Solution Value
 * @return size_t
 */
void Layout::set_solution_uy(size_t const val)
{
    m_logger->add_solution_die_uy(val);
    m_solutions_uy.push_back(val);
}

/**
 * @brief Get index of best solution
 * 
 * @return size_t
 */
size_t Layout::get_idx_best_solution()
{
    assertion_check (m_solutions_uy.size() == m_solutions_ux.size());

    size_t idx = 0;
    double best_area  = std::numeric_limits<double>::max();
    
    for(size_t i = 0; i < m_solutions_uy.size(); ++i){
        size_t x = m_solutions_ux[i];
        size_t y = m_solutions_uy[i];

        double tmp = ((double)x * (double)y)/1000000;

        if(tmp < best_area){
            best_area = tmp;
            idx = i;
        }
    }
    return idx;
}

/**
 * @brief Check if a particular solution exists
 * 
 * @param solution Solution ID to check
 * @return bool
 */
bool Layout::has_solution(int const solution)
{
    assertion_check (m_solutions_ux.size() == m_solutions_ux.size());

    return m_solutions_ux.size() > solution;
}

/**
 * @brief Set clauses for ux
 * 
 * @param formula UX Clauses
 */
void Layout::set_formula_ux (z3::expr const & formula)
{
    m_ux = formula;
}

/**
 * @brief Set clauses for uy
 * 
 * @param formula UY Clauses
 */
void Layout::set_formula_uy (z3::expr const & formula)
{
    m_uy = formula;
}

/**
 * @brief  Get LY as Unsigned Integer
 * 
 * @return size_t
 */
size_t Layout::get_ly_numerical()
{
    assertion_check (m_ly.is_numeral());

    return m_ly.get_numeral_uint();
}

/**
 * @brief  Get LX as Unsigned Integer
 * 
 * @return size_t
 */
size_t Layout::get_lx_numerical()
{
    assertion_check (m_lx.is_numeral());

    return m_lx.get_numeral_uint();
}

/**
 * @brief  Get UX as Unsigned Integer
 * 
 * @return size_t
 */
size_t Layout::get_ux_numercial()
{
    assertion_check (m_ux.is_numeral());

    return m_ux.get_numeral_uint();
}

/**
 * @brief Get UY as Unsigned Integer
 * 
 * @return size_t
 */
size_t Layout::get_uy_numerical()
{
    assertion_check (m_uy.is_numeral());

    return m_uy.get_numeral_uint();
}
