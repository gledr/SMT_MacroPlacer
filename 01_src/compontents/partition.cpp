//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : partition.cpp
//
// Date         : 24. February 2020
// Compiler     : gcc version 9.2.1 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Partition Component
//==================================================================
#include "partition.hpp"

#include <iterator>

using namespace Placer;
using namespace Placer::Utils;

std::string Partition::m_partition_keyword = "partition";

/**
 * @brief Constructor
 * 
 * @param z3_ctx Z3 Context
 */
Partition::Partition():
    Component(),
    m_ux(m_encode->get_value(0)),
    m_uy(m_encode->get_value(0)),
    m_components_in_partition(m_encode->get_value(0)),
    m_encode_pin_partition_frontier(m_encode->get_value(0)),
    m_encode_pins_not_overlapping(m_encode->get_value(0)),
    m_encode_partition(m_encode->get_value(0)),
    m_key(m_key_counter++)
{
    m_id = std::to_string(this->get_partition_id());
    m_ly_id = m_partition_keyword + "_" + m_id + "_ly";
    m_lx_id = m_partition_keyword + "_" + m_id + "_lx";
    m_uy_id = m_partition_keyword + "_" + m_id + "_uy";
    m_ux_id = m_partition_keyword + "_" + m_id + "_ux";
    m_orientation_id = m_partition_keyword + "_" + m_id + "_orientation";

    m_lx = m_encode->get_constant(m_lx_id);
    m_ly = m_encode->get_constant(m_ly_id);
    m_ux = m_encode->get_constant(m_ux_id);
    m_uy = m_encode->get_constant(m_uy_id);
    m_orientation = m_encode->get_constant(m_orientation_id);

    m_has_value_lx = false;
    m_has_value_ly = false;
    m_has_value_ux = false;
    m_has_value_uy = false;
}

/**
 * @brief Destructor
 */
Partition::~Partition()
{
}

/**
 * @brief Add Macro to Partition
 * 
 * @param macro Pointer to Macro
 */
void Partition::add_macro(Macro* macro)
{
    nullpointer_check (macro);

    m_components.push_back(macro);
}

/**
 * @brief Add Macros to Partition
 * 
 * @param macros Vector to Macro-Pointer
 */
void Partition::add_macros(std::vector<Macro *> macros)
{
    for (auto itor: macros){
        m_components.push_back(itor);
    }
}

/**
 * @brief Add SubPartition
 * 
 * @param partition Pointer to Subpartition
 */
void Partition::add_subpartition(Partition* subpartition)
{
    nullpointer_check (subpartition);

    m_components.push_back(subpartition);
}

/**
 * @brief Add SubPartitions
 * 
 * @param partitions Vector to Partition Pointer
 */
void Partition::add_subparititions(std::vector<Partition *> subpartitions)
{
    for (auto itor: subpartitions){
        m_components.push_back(itor);
    }
}

/**
 * @brief Calculate White Space Percentage in Partition
 */
void Partition::calculate_white_space()
{
    // 1. Calculate Area of Partition
    size_t i = m_ux.get_numeral_uint();
    size_t j = m_uy.get_numeral_uint();
    m_partion_area = i * j;

    // 2. Calculate Minimum Area needed by Macros
    m_macro_area = 0;
    for(auto& m: m_components){
        m_macro_area += m->get_area();
    }
}

/**
 * @brief Get current white space percentage
 * 
 * @return double
 */
double Partition::get_white_space_percentage()
{
    this->calculate_white_space();

    //std::cout << "Minimum Size: " << m_macro_area << std::endl;
    //std::cout << "Partition Size: " << m_partion_area << std::endl;

    return 100.0 - (((double)m_macro_area / (double)m_partion_area) * 100);
}

/**
 * @brief Check if Partition has white space
 * 
 * @return bool
 */
bool Partition::has_white_space()
{
    return m_macro_area > m_partion_area;
}

/**
 * @brief Get Access to Components
 * 
 * @return std::vector< Placer::Components* >&
 */
std::vector<Component*>& Partition::get_components()
{
    return m_components;
}

/**
 * @brief Get UX Constraints
 * 
 * @return z3::expr
 */
z3::expr Partition::get_ux()
{
    return m_ux;
}

/**
 * @brief Get UY Constraints
 * 
 * @return z3::expr
 */
z3::expr Partition::get_uy()
{
    return m_uy;
}

/**
 * @brief Free LX Variable
 * 
 */
void Partition::free_lx()
{
    m_lx = m_encode->get_constant(m_lx_id);
    m_has_value_lx = false;
}

/**
 * @brief Free LY Variable
 */
void Partition::free_ly()
{
    m_ly = m_encode->get_constant(m_ly_id);
    m_has_value_ly = false;
}

/**
 * @brief Free UX Variable
 */
void Partition::free_ux()
{
    m_ux = m_encode->get_constant(m_ux_id);
    m_has_value_ux = false;
}

/**
 * @brief Free UY Variable
 */
void Partition::free_uy()
{
    m_uy = m_encode->get_constant(m_uy_id);
    m_has_value_uy = false;
}

/**
 * @brief Set Value for LX Constraints
 * 
 * @param val
 */
void Partition::set_lx(size_t const val)
{
    m_lx = m_encode->get_value(val);
    m_has_value_lx = true;
}

/**
 * @brief Set Value for LY Constraints
 * 
 * @param val
 */
void Partition::set_ly(size_t const val)
{
    m_ly = m_encode->get_value(val);
    m_has_value_ly = true;
}

/**
 * @brief Set Value for UX Constraints
 *
 * @param val 
 */
void Partition::set_ux(size_t const val)
{
    m_ux = m_encode->get_value(val);
    m_has_value_ux = true;
}

/**
 * @brief Set Value for UY Constraints
 * 
 * @param val
 */
void Partition::set_uy(size_t const val)
{
    m_uy = m_encode->get_value(val);
    m_has_value_uy = true;
}

/**
 * @brief Set Height Constraints
 * 
 * @param val
 */
void Partition::set_height(size_t const val)
{
    m_height = m_encode->get_value(val);
}

/**
 * @brief Set Widht Constraints
 * 
 * @param val
 */
void Partition::set_witdh(size_t const val)
{
    m_width = m_encode->get_value(val);
}

/**
 * @brief Free Orientation Variable
 */
void Partition::free_orientation()
{
    m_orientation = m_encode->get_constant(m_orientation_id);
}

/**
 * @brief Set Orientation Constraints
 * 
 * @param val
 */
void Partition::set_orientation(size_t const val)
{
    m_orientation = m_encode->get_value(val);
}

/**
 * @brief Check if LX is free or hold a value
 * 
 * @return bool
 */
bool Partition::has_value_lx()
{
    return m_has_value_lx;
}

/**
 * @brief Check if LY is free or hold a value
 * 
 * @return bool
 */
bool Partition::has_value_ly()
{
    return m_has_value_ly;
}

/**
 * @brief Check if UX is free or hold a value
 * 
 * @return bool
 */
bool Partition::has_value_ux()
{
    return m_has_value_ux;
}

/**
 * @brief Check if UY is free or hold a value
 * 
 * @return bool
 */
bool Partition::has_value_uy()
{
    return m_has_value_uy;
}

/**
 * @brief Get area occupied by partition
 * 
 * @return size_t
 */
size_t Partition::get_area()
{
    size_t i = m_ux.get_numeral_uint();
    size_t j = m_uy.get_numeral_uint();
    m_partion_area = i * j;
    
    return m_partion_area;
}

/**
 * @brief Dump Partition Information to given stream
 * 
 * @param stream Stream to dump to
 */
void Partition::dump(std::ostream& stream)
{
    stream << "======================" << std::endl;
    stream << "Partition: " << m_id << std::endl;
    stream << "(" << m_lx << ":" << m_ly << ") (" << m_ux << ":" << m_uy << ")" << std::endl;
    stream << "h: " << m_height << " w: " << m_width << std::endl;
    stream << "======================" << std::endl;
}

/**
 * @brief Get Access to the Partitions Pins
 * 
 * @return std::vector< Placer::Pin* >
 */
std::vector<Pin*> Partition::get_pins()
{
    std::vector<Pin*> retval;
    std::transform(m_pins.begin(), m_pins.end(), std::back_inserter(retval),
         [](std::pair<std::string, Pin*> const & p) {
             return p.second; 
        });
    
    return retval;
}

/**
 * @brief Push Up Pins from Inner Components
 */
void Partition::push_up_pins()
{
    for (Component* c: m_components){
        for(Pin* p: c->get_pins()){
            m_pins[c->get_id() + ":" + p->get_name()] =  p;
        }
    }
}

/**
 * @brief Encode Macro Pins not overlapping
 */
void Partition::encode_pins_non_overlapping(eRotation const type)
{
    z3::expr_vector clauses(m_z3_ctx);
    
    std::vector<Pin*> pins = this->get_pins();
    
    for (size_t i = 0; i < m_pins.size(); ++i){
        for (size_t j = 0; j < m_pins.size(); ++j){
            if (i == j){
                continue;
            } else {
                Pin* a = pins[i];
                Pin* b = pins[j];
                
                z3::expr_vector clause(m_z3_ctx);
                //
                // Use Case North as default since pins are already encoded 
                // to the frontier
                //
                z3::expr left = m_encode->lt(a->get_pin_pos_x(), b->get_pin_pos_x());
                z3::expr right = m_encode->gt(a->get_pin_pos_x(), b->get_pin_pos_x());
                z3::expr upper = m_encode->gt(a->get_pin_pos_y(), b->get_pin_pos_y());
                z3::expr lower = m_encode->lt(a->get_pin_pos_y(), b->get_pin_pos_y());
                
                clause.push_back(left);
                clause.push_back(right);
                clause.push_back(upper);
                clause.push_back(lower);
                clauses.push_back(z3::mk_or(clause));
            }
        }
    }
    m_encode_pins_not_overlapping = z3::mk_and(clauses);
}

/**
 * @brief Encode the Pins of a Macro to be located on its frontier
 * 
 * The Pins may be located on the UPPER, LEFT, LOWER or RIGHT plane
 */
void Partition::encode_pins_on_partition_frontier(eRotation const type)
{
    try {
        z3::expr_vector clauses(m_z3_ctx);

        for(auto pin: m_pins){
            z3::expr x = pin.second->get_pin_pos_x();
            z3::expr y = pin.second->get_pin_pos_y();
                
///{{{      Case N
            z3::expr_vector case_n(m_z3_ctx);
{
            //LEFT PLANE
            z3::expr_vector case_n1(m_z3_ctx);
            case_n1.push_back(x == m_lx);
            case_n1.push_back(y > m_ly);
            case_n1.push_back(y < (m_ly + m_height));
            case_n.push_back(z3::mk_and(case_n1));

            //RIGHT PLANE
            z3::expr_vector case_n2(m_z3_ctx);
            case_n2.push_back(x == (m_lx + m_width));
            case_n2.push_back(y > m_ly);
            case_n2.push_back(y < (m_ly + m_height));
            case_n.push_back(z3::mk_and(case_n2));

            //LOWER PLANE
            z3::expr_vector case_n3(m_z3_ctx);
            case_n3.push_back(y == m_ly);
            case_n3.push_back(x > m_lx);
            case_n3.push_back(x < (m_lx + m_width));
            case_n.push_back(z3::mk_and(case_n3));

            //UPPER PLANE
            z3::expr_vector case_n4(m_z3_ctx);
            case_n4.push_back(y == (m_ly + m_height));
            case_n4.push_back(x > m_lx);
            case_n4.push_back(x < (m_lx + m_width));
            case_n.push_back(z3::mk_and(case_n4));
}
///}}}
///{{{      Case W
            z3::expr_vector case_w(m_z3_ctx);
{
            // Case 1 x = moveable, y = ly LEFT PLANE
            z3::expr_vector case_w1(m_z3_ctx);
            case_w1.push_back(y == (m_ly - m_width));
            case_w1.push_back(x < m_lx);
            case_w1.push_back(x > (m_lx - m_height));
            case_w.push_back(z3::mk_and(case_w1));

            // Case 2 x = moveable, y = uy RIGHT PLANE
            z3::expr_vector case_w2(m_z3_ctx);
            case_w2.push_back(y == m_ly);
            case_w2.push_back(x < m_lx);
            case_w2.push_back(x > (m_lx - m_height));
            case_w.push_back(z3::mk_and(case_w2));

            // Case 3 x = lx, y = moveable LOWER PLANE
            z3::expr_vector case_w3(m_z3_ctx);
            case_w3.push_back(x == m_lx);
            case_w3.push_back(y > m_ly);
            case_w3.push_back(y < (m_ly + m_width));
            case_w.push_back(z3::mk_and(case_w3));

            // Case 4 x = ux, y = moveable UPPER PLANE
            z3::expr_vector case_w4(m_z3_ctx);
            case_w4.push_back(x == (m_lx + m_height));
            case_w4.push_back(y > m_ly);
            case_w4.push_back(y < (m_ly + m_width));
            case_w.push_back(z3::mk_and(case_w4));
}
///}}}
///{{{      Case S
            z3::expr_vector case_s(m_z3_ctx);
{
            // Case 1 x = moveable, y = ly LEFT PLANE
            z3::expr_vector case_s1(m_z3_ctx);
            case_s1.push_back(y == (m_ly - m_height));
            case_s1.push_back(x < m_lx);
            case_s1.push_back(x > (m_lx - m_width));
            case_s.push_back(z3::mk_and(case_s1));

            // Case 2 x = moveable, y = uy RIGHT PLANE
            z3::expr_vector case_s2(m_z3_ctx);
            case_s2.push_back(y == (m_ly + m_height)); 
            case_s2.push_back(x < m_lx);
            case_s2.push_back(x > (m_lx - m_width));
            case_s.push_back(z3::mk_and(case_s2));

            // Case 3 x = lx, y = moveable LOWER PLANE
            z3::expr_vector case_s3(m_z3_ctx);
            case_s3.push_back(x == (m_lx - m_width));
            case_s3.push_back(y < m_ly);
            case_s3.push_back(y > (m_ly - m_height));
            case_s.push_back(z3::mk_and(case_s3));
            
            // Case 4 x = ux, y = moveable UPPER PLANE
            z3::expr_vector case_s4(m_z3_ctx);
            case_s4.push_back(x == m_lx);
            case_s4.push_back(y < m_ly);
            case_s4.push_back(y > (m_ly - m_height));
            case_s.push_back(z3::mk_and(case_s4));
}
///}}}
///{{{      Case E
            z3::expr_vector case_e(m_z3_ctx);
{
            // Case 1 x = moveable, y = ly LEFT PLANE
            z3::expr_vector case_e1(m_z3_ctx);
            case_e1.push_back(y == m_ly);
            case_e1.push_back(x > m_lx);
            case_e1.push_back(x < (m_lx + m_height));
            case_e.push_back(z3::mk_and(case_e1));

            // Case 2 x = moveable, y = uy RIGHT PLANE
            z3::expr_vector case_e2(m_z3_ctx);
            case_e2.push_back(y == (m_ly + m_width));
            case_e2.push_back(x > m_lx);
            case_e2.push_back(x < (m_lx + m_height));
            case_e.push_back(z3::mk_and(case_e2));

            // Case 3 x = lx, y = moveable LOWER PLANE
            z3::expr_vector case_e3(m_z3_ctx);
            case_e3.push_back(x == (m_lx - m_height));
            case_e3.push_back(y > m_ly);
            case_e3.push_back(y < (m_ly + m_width));
            case_e.push_back(z3::mk_and(case_e3));

            // Case 4 x = ux, y = moveable UPPER PLANE
            z3::expr_vector case_e4(m_z3_ctx);
            case_e4.push_back(x == (m_lx + m_height));
            case_e4.push_back(y > m_ly);
            case_e4.push_back(y < (m_ly + m_width));
            case_e.push_back(z3::mk_and(case_e4));
}
///}}}
///}}}
           if (type == e4D){
                z3::expr clause = z3::ite(this->is_N(), z3::mk_or(case_n),
                                z3::ite(this->is_W(), z3::mk_or(case_w),
                                z3::ite(this->is_S(), z3::mk_or(case_s),
                                z3::ite(this->is_E(), z3::mk_or(case_e), m_z3_ctx.bool_val(false)))));
                clauses.push_back(clause);
           } else if (type == e2D){
                z3::expr clause = z3::ite(this->is_N(), z3::mk_or(case_n),
                                  z3::ite(this->is_W(), z3::mk_or(case_w), m_z3_ctx.bool_val(false)));
           } else {
                notsupported_check("Only Rotation 2D and 4D are supported!");
           }
        }
        
        m_encode_pin_partition_frontier = z3::mk_and(clauses);
    } catch (z3::exception const & exp){
        throw PlacerException(exp.msg());
    }
}

void Partition::encode_partition()
{
    std::cout << "Encode Partition: " << m_key << std::endl;
    
    z3::expr_vector clauses(m_z3_ctx);

    this->encode_pins_non_overlapping(e2D);
    this->encode_pins_on_partition_frontier(e2D);

    clauses.push_back(m_encode_pin_partition_frontier);
    clauses.push_back(m_encode_pins_not_overlapping);

    m_encode_partition = z3::mk_and(clauses);
}

z3::expr Partition::get_partition_constraints()
{
    return m_encode_partition;
}

size_t Partition::get_key()
{
    return m_key;
}
