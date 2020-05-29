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
    m_components_in_partition(m_encode->get_value(0))
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
//     nullpointer_check (macro);
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
