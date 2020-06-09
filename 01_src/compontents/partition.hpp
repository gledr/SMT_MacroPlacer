//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : partition.hpp
//
// Date         : 24. February 2020
// Compiler     : gcc version 9.2.1 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Partition Component
//==================================================================
#ifndef PARTITION_HPP
#define PARTITION_HPP

#include <component.hpp>
#include <macro.hpp>
#include <exception.hpp>

#include <vector>
#include <iterator>

namespace Placer {

/**
 * @class Partition
 * 
 * @brief Layout Partition
 */ 
class Partition: public virtual Component {
public:
    Partition();

    virtual ~Partition();

    void add_macro(Macro* macro);
    void add_macros(std::vector<Macro*> macros);
    
    void add_subpartition(Partition* subpartition);
    void add_subparititions(std::vector<Partition*> subpartitions);

    std::vector<Component*>& get_components();

    virtual size_t get_area();
    virtual bool is_free();
    double get_white_space_percentage();
    bool has_white_space();

    bool has_value_lx();
    bool has_value_ly();
    bool has_value_ux();
    bool has_value_uy();

    z3::expr get_ux();
    z3::expr get_uy();
    z3::expr get_partition_constraints();
    size_t get_key();

    void set_ux(size_t const val);
    void set_uy(size_t const val);
    void set_ly(size_t const val);
    void set_lx(size_t const val);
    void set_orientation(size_t const val);

    void free_ux();
    void free_uy();
    void free_ly();
    void free_lx();
    void free_orientation();

    void set_witdh(size_t const val);
    void set_height(size_t const val);

    virtual void dump(std::ostream & stream = std::cout);

    virtual std::vector<Pin*> get_pins();

    void push_up_pins();
    void encode_partition();

private:
    std::vector<Component*> m_components;
    std::map<std::string, Pin*> m_pins;
    z3::expr m_ux;
    z3::expr m_uy;
    size_t m_key;

    bool m_has_value_lx;
    bool m_has_value_ly;
    bool m_has_value_ux;
    bool m_has_value_uy;

    static std::string m_partition_keyword;
    std::string m_lx_id;
    std::string m_ly_id;
    std::string m_ux_id;
    std::string m_uy_id;
    std::string m_orientation_id;

    z3::expr m_components_in_partition;

    size_t m_partion_area;
    size_t m_macro_area;
    std::string m_id;

    void calculate_white_space();

    void encode_pins_on_partition_frontier(eRotation const type);
    void encode_pins_non_overlapping(eRotation const type);

    z3::expr m_encode_partition;
    z3::expr m_encode_pin_partition_frontier;
    z3::expr m_encode_pins_not_overlapping;
};

} /* namespace Placer */

#endif /* PARTITION_HPP */
