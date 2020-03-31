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

Layout::Layout (z3::context* z3_ctx):
    Object(),
    m_lx(z3_ctx->int_const(std::string("die_lx").c_str())),
    m_ly(z3_ctx->int_const(std::string("die_uy").c_str())),
    m_ux(z3_ctx->int_const(std::string("die_ux").c_str())),
    m_uy(z3_ctx->int_const(std::string("die_uy").c_str())),
    m_units(z3_ctx->int_const(std::string("die_units").c_str())),
    m_free_lx(true),
    m_free_ly(true),
    m_free_ux(true),
    m_free_uy(true)
{
    assert(z3_ctx != nullptr);
    m_z3_ctx = z3_ctx;
    
    m_logger = Logger::getInstance();
}

Layout::~Layout ()
{
    m_z3_ctx = nullptr;
}

double Layout::get_min_die_prediction()
{
    return m_min_die_size;
}

void Layout::set_min_die_predition(double const & die_size)
{
    m_min_die_size = die_size;
}

z3::expr& Layout::get_lx()
{
    return m_lx;
}

z3::expr& Layout::get_ux()
{
    return m_ux;
}

z3::expr& Layout::get_ly()
{
    return m_ly;
}

z3::expr& Layout::get_uy()
{
    return m_uy;
}

z3::expr& Layout::get_units()
{
    return m_units;
}

void Layout::free_ux ()
{
    m_ux = m_z3_ctx->int_const(std::string("die_ux").c_str());
    m_free_ux = true;
}

void Layout::free_uy ()
{
    m_uy = m_z3_ctx->int_const(std::string("die_uy").c_str());
    m_free_uy = true;
}

void Layout::free_lx ()
{
    m_lx = m_z3_ctx->int_const(std::string("die_lx").c_str());
    m_free_lx = true;
}

void Layout::free_ly ()
{
    m_ly = m_z3_ctx->int_const(std::string("die_ly").c_str());
    m_free_ly = true;
}

void Layout::set_lx(size_t const val)
{
    m_free_lx = false;
    m_lx = m_z3_ctx->int_val(val);
}

void Layout::set_ly(size_t const val)
{
    m_free_ly = false;
    m_ly = m_z3_ctx->int_val(val);
}

void Layout::set_ux(size_t const val)
{
    m_logger->set_die_ux(val);
    m_free_ux = false;
    m_ux = m_z3_ctx->int_val(val);
}

void Layout::set_uy(size_t const val)
{
    m_logger->set_die_uy(val);
    m_free_uy = false;
    m_uy = m_z3_ctx->int_val(val);
}

void Layout::set_units(size_t const val)
{
      m_units = m_z3_ctx->int_val(val);
}

bool Layout::is_free_lx()
{
    return m_free_lx;
}

bool Layout::is_free_ly()
{
    return m_free_ly;
}

bool Layout::is_free_uy()
{
    return m_free_uy;
}

bool Layout::is_free_ux()
{
    return m_free_ux;
}

size_t Layout::get_solution_ux(size_t const idx)
{
    return m_solutions_ux[idx];
}

size_t Layout::get_solution_uy(size_t const idx)
{
    return m_solutions_uy[idx];
}

void Layout::set_solution_ux(size_t const val)
{
    
    m_solutions_ux.push_back(val);
}

void Layout::set_solution_uy(size_t const val)
{
    m_solutions_uy.push_back(val);
}

size_t Layout::get_idx_best_solution()
{
    assert (m_solutions_uy.size() == m_solutions_ux.size());
    
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

void Layout::set_formula_ux (z3::expr const & formula)
{
    m_ux = formula;
}

void Layout::set_formula_uy (z3::expr const & formula)
{
    m_uy = formula;
}
