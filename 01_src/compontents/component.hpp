//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : component.hpp
//
// Date         : 14. March 2020
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Super Class for Macros/Cells and Partitions
//==================================================================
#ifndef COMPONENT_HPP
#define COMPONENT_HPP

#include <object.hpp>
#include <encoding_utils.hpp>
#include <exception.hpp>
#include <pin.hpp>

#include <z3++.h>

namespace Placer {

/**
 * @class Component
 * 
 * @brief Superclass for Macro/Cell/Partition
 */
class Component: public virtual Object {
public: 
    ~Component();

    z3::expr get_lx(eOrientation const orientation);
    z3::expr get_ly(eOrientation const orientation);
    z3::expr get_ux(eOrientation const orientation);
    z3::expr get_uy(eOrientation const orientation);
    
    std::stringstream _get_lx(eOrientation const orientation);
    std::stringstream _get_ly(eOrientation const orientation);
    std::stringstream _get_ux(eOrientation const orientation);
    std::stringstream _get_uy(eOrientation const orientation);
    
    size_t get_lx_numeral();
    size_t get_ly_numeral();

    z3::expr& get_lx();
    z3::expr& get_ly();
    z3::expr& get_orientation();
    z3::expr& get_width();
    z3::expr& get_height();

    size_t get_width_numeral();
    size_t get_height_numeral();

    std::string get_name();
    std::string get_id();

    z3::expr is_N();
    z3::expr is_W();
    z3::expr is_S();
    z3::expr is_E();

    std::string _is_N();
    std::string _is_W();
    std::string _is_S();
    std::string _is_E();

    bool has_solution(size_t const id);

    void add_solution_lx(size_t const lx);
    void add_solution_ly(size_t const ly);
    void add_solution_orientation(Placer::eOrientation const orientation);
    
    size_t get_solution_lx(size_t const id);
    size_t get_solution_ly(size_t const id);
    eOrientation get_solution_orientation(size_t const id);

    virtual size_t get_area() = 0;
    virtual void dump(std::ostream & stream = std::cout) = 0;
    virtual bool is_free() = 0;
    
    virtual std::vector<Pin*> get_pins() = 0;

protected:
    Component ();

    EncodingUtils* m_encode;

    std::string m_name;
    std::string m_id;

    z3::expr m_lx;
    z3::expr m_ly;
    z3::expr m_orientation;
    z3::expr m_width;
    z3::expr m_height;
    bool m_free;

    std::vector<size_t> m_sol_lx;
    std::vector<size_t> m_sol_ly;
    std::vector<eOrientation> m_sol_orientation;
};

} /* namespace Placer */

#endif /* COMPONENT_HPP */
