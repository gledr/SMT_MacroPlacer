//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : edge.cpp
//
// Date         : 08. January 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Macro Dependency Tree Edge
//==================================================================
#include "edge.hpp"
#include "node.hpp"

using namespace Placer;

static bool debug = false;

/**
 * @brief Constructor
 * 
 * @param from Source Node
 * @param to Target Node
 * @param from_pin Source Pin
 * @param to_pin Target Pin
 * @param edge_name Edge Identifier
 */
Edge::Edge(Node* from, Node* to,
           std::string const & from_pin,
           std::string const & to_pin,
           std::string const & edge_name):
    m_name(edge_name)
{
    assert (from != nullptr);
    assert (to != nullptr); 
    //assert (from != to); FIXME
    
    // Case I: Input 
    if(from->is_terminal() &&
       from->get_terminal()->is_input() &&
       to->is_node()){
        m_from = from;
        m_to = to;
        m_from_pin = from_pin;
        m_to_pin = to_pin;
        
    // Case II: Output
    } else if (from->is_terminal() && 
               from->get_terminal()->is_output() &&
               to->is_node()){
        m_from = to;
        m_to = from;
        m_from_pin = to_pin;
        m_to_pin = from_pin;
    } else {

        m_from = from;
        m_to = to;
        m_from_pin = from_pin;
        m_to_pin = to_pin;
    }
    
    debug && std::cout << "[Info]: New Edge " << m_from->get_id()
                       << " -> " << m_to->get_id() << std::endl;
}

/**
 * @brief Destructor
 */
Edge::~Edge()
{
    m_from = nullptr;
    m_to = nullptr;
}

/**
 * @brief Get Edge Identifier
 * 
 * @return std::string
 */
std::string Edge::get_name()
{
    return m_name;
}

/**
 * @brief Dump Edge Information
 * 
 * @param stream Target Steam to dump
 */
void Edge::dump(std::ostream& stream)
{
    if(this->is_terminal_to_macro()){
        stream << "Edge (" << this->m_name << ") " 
               << m_from->get_terminal()->get_id() 
               << " -> " << m_to->get_macro()->get_id() << std::endl;
    } else if (this->is_terminal_to_cell()){
        stream << "Edge (" << this->m_name << ") " 
               << m_from->get_terminal()->get_id()
               << " -> " << m_to->get_cell()->get_id() << std::endl;
    } else if (this->is_macro_to_macro()){
        stream << "Edge (" << this->m_name << ") "
               << m_from->get_macro()->get_id()
               << " -> " << m_to->get_macro()->get_id() << std::endl;
    } else if (this->is_macro_to_cell()){
        stream << "Edge (" << this->m_name << ") "
               << m_from->get_macro()->get_id()
               << " -> " << m_to->get_cell()->get_id() << std::endl;
    } else if (this->is_cell_to_macro()){
        stream << "Edge (" << this->m_name << ") "
               << m_from->get_cell()->get_id()
               << " -> " << m_to->get_macro()->get_id() << std::endl;
    } else if (this->is_macro_to_terminal()){
         stream << "Edge (" << this->m_name << ") "
                << m_from->get_macro()->get_id()
                << " -> " << m_to->get_terminal()->get_id() << std::endl;
    } else if (this->is_cell_to_terminal()){
        stream << "Edge (" << this->m_name << ") "
               << m_from->get_cell()->get_id() 
               << " -> " << m_to->get_terminal()->get_id() << std::endl;
    } else {
        assert (0);
    }
}

/**
 * @brief Get Source Node
 * 
 * @return Placer::Node*
 */
Node* Edge::get_from()
{
    return m_from;
}

/**
 * @brief Get Target Node
 * 
 * @return Placer::Node*
 */
Node* Edge::get_to()
{
    return m_to;
}

/**
 * @brief Check if Edge is from Cell to Macro
 * 
 * @return bool
 */
bool Edge::is_cell_to_macro()
{
    return m_from->has_cell() && m_to->has_macro();
}

/**
 * @brief Check if Edge if from Macro to Cell
 * 
 * @return bool
 */
bool Edge::is_macro_to_cell()
{
    return m_from->has_macro() && m_to->has_cell();
}

/**
 * @brief Check if Edge is from Macro to Macro
 * 
 * @return bool
 */
bool Edge::is_macro_to_macro()
{
    return m_from->has_macro() && m_to->has_macro();
}

/**
 * @brief Check uf Edge is from Terminal to Cell
 * 
 * @return bool
 */
bool Edge::is_terminal_to_cell()
{
    return m_from->is_terminal() && m_to->has_cell();
}

/**
 * @brief Check if Edge is from Terminal to Macro
 * 
 * @return bool
 */
bool Edge::is_terminal_to_macro()
{
    return m_from->is_terminal() && m_to->has_macro();
}

/**
 * @brief Check if Edge is from Cell to Terminal
 * 
 * @return bool
 */
bool Edge::is_cell_to_terminal()
{
    return m_from->has_cell() && m_to->is_terminal();
}

/**
 * @brief Check if Edge is from Macro to Terminal
 * 
 * @return bool
 */
bool Edge::is_macro_to_terminal()
{
    return m_from->has_macro() && m_to->is_terminal();
}
