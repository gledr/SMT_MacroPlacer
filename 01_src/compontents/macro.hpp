//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : macro.hpp
//
// Date         : 22. December 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Macro Component
//==================================================================
#ifndef MACRO_HPP
#define MACRO_HPP

#include <algorithm>

#include <component.hpp>
#include <lefdefIO.h>
#include <layout.hpp>
#include <pin.hpp>
#include <supplement.hpp>

namespace Placer {

/**
 * @class Macro
 * @brief Symbolic Representation of a Macro for Encoding
 */
class Macro: public virtual Component {
public:

    Macro(std::string const & name,
          std::string const & id,
          size_t const widht,
          size_t const height);

   Macro(std::string const & name,
         std::string const & id,
         size_t const width,
         size_t const height,
         size_t const pos_x,
         size_t const pos_y,
         size_t const orientation);
   
   Macro(std::string const & name,
         std::string const & id,
         size_t const width,
         size_t const height,
         size_t const layout_x,
         size_t const layout_y);

    virtual ~Macro();
    
    void init_grid();
    z3::expr encode_grid();
    z3::expr_vector get_grid_costs();
    z3::expr_vector get_grid_coordinates();
    
    void set_supplement(Supplement* supplement);

    void add_pin(Pin* const pin);
    Pin* get_pin(std::string const & id);

    virtual size_t get_area();
    virtual void dump(std::ostream & stream = std::cout);

    std::vector<Pin*> get_pins();
    bool is_free();

    size_t get_solution_lx(size_t const id);
    size_t get_solution_ly(size_t const id);
    size_t get_solution_orientation(size_t const id);

    void encode_pins();
    z3::expr get_pin_constraints();

private:
    std::map<std::string, Pin*> m_pins;
    Pin* m_active_pin;
    Supplement* m_supplement;

    bool m_free;
    
    z3::expr_vector m_root_coordinate;
    z3::expr_vector m_grid_coordinates;
    z3::expr_vector m_cost_distribution;
    
    size_t m_layout_x;
    size_t m_layout_y;
    
    void handle_supplement();
    
    /**
     * SMT Encoding
     */
    void encode_pins_on_macro_frontier();
    void encode_pins_non_overlapping();
    void encode_pins_center_of_macro();
    
    z3::expr m_encode_pin_macro_frontier;
    z3::expr m_encode_pins_not_overlapping;
    z3::expr m_encode_pins_center_of_macro;
};

}

#endif /*  MACRO_HPP */
