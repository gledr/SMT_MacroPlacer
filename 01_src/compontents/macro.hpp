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
#include <layout.hpp>
#include <pin.hpp>
#include <supplement.hpp>

namespace Placer {

class Partition;

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

    virtual ~Macro();

    bool operator== (Macro const & m);

    void set_supplement(Supplement* supplement);

    void add_pin(Pin* const pin);
    Pin* get_pin(std::string const & id);
    bool has_pin(std::string const & id);

    virtual size_t get_area();
    virtual void dump(std::ostream & stream = std::cout);

    void set_parent_partition(Partition* const parent);
    Partition* get_parent_partition();

    virtual std::vector<Pin*> get_pins();
    virtual bool is_free();
    bool is_part_of_partition();
    size_t get_key();

    void add_solution_root(size_t const x, size_t const y);
    std::pair<size_t, size_t> get_solution_root() const;

    void add_solution_grid(size_t const x, size_t const y);

    void encode_pins();
    z3::expr get_pin_constraints();

private:
    std::map<std::string, Pin*> m_pins;
    Pin* m_active_pin;
    Supplement* m_supplement;
    Utils::Logger* m_logger;
    Partition* m_parent;

    size_t m_key;

    void handle_supplement();
    void resolve_pin_position();

    /**
     * SMT Encoding
     */
    void encode_pins_on_macro_frontier(eRotation const rotation);
    void encode_pins_non_overlapping();
    void encode_pins_center_of_macro(eRotation const rotation);
    void encode_pins_relative_to_center(eRotation const rotation);

    z3::expr m_pin_constraints;
    z3::expr m_encode_pin_macro_frontier;
    z3::expr m_encode_pins_not_overlapping;
    z3::expr m_encode_pins_center_of_macro;
    z3::expr m_encode_pins_relative_to_center;
};

} /* namespace Placer */

#endif /*  MACRO_HPP */
