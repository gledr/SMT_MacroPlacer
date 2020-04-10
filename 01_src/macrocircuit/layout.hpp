//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : layout.hpp
//
// Date         : 14. January 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Die Layout
//==================================================================
#ifndef LAYOUT_HPP
#define LAYOUT_HPP

#include <object.hpp>
#include <logger.hpp>
#include <encoding_utils.hpp>

#include <string>
#include <iostream>
#include <algorithm>
#include <limits>

#include <z3++.h>

namespace Placer {

/**
 * @class Layout
 * @brief Layout for MacroCircuit
 */
class Layout: public virtual Object {
public:

    Layout ();

    virtual ~Layout ();

    void free_ux();
    void free_uy();
    void free_lx();
    void free_ly();

    z3::expr& get_lx();
    z3::expr& get_ux();
    z3::expr& get_ly();
    z3::expr& get_uy();
    z3::expr& get_units();

    void set_lx(size_t const val);
    void set_ly(size_t const val);
    void set_ux(size_t const val);
    void set_uy(size_t const val);
    void set_units(size_t const val);

    void set_formula_ux(z3::expr const & formula);
    void set_formula_uy(z3::expr const & formula);

    void set_solution_ux(size_t const val);
    void set_solution_uy(size_t const val);

    size_t get_solution_ux(size_t const idx);
    size_t get_solution_uy(size_t const idx);

    bool is_free_lx();
    bool is_free_ly();
    bool is_free_uy();
    bool is_free_ux();

    size_t get_idx_best_solution();

    void set_min_die_predition(double const & val);
    double get_min_die_prediction();

private:

    Utils::Logger* m_logger;
    double m_min_die_size;
    EncodingUtils* m_encode;

    bool m_free_lx;
    bool m_free_ly;
    bool m_free_ux;
    bool m_free_uy;

    std::vector<size_t> m_solutions_ux;
    std::vector<size_t> m_solutions_uy;

    z3::context* m_z3_ctx;
    z3::expr    m_lx;
    z3::expr    m_ly;
    z3::expr    m_ux;
    z3::expr    m_uy;
    z3::expr    m_units;
};

}

#endif /* LAYOUT_HPP */
