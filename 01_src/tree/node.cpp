//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : node.cpp
//
// Date         : 27. December 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Macro Dependency Tree Node
//==================================================================
#include "node.hpp"

using namespace Placer;
using namespace Placer::Utils;

static bool debug = false;

/**
 * @brief Constructor
 * 
 * @param _macro Node is Macro
 */
Node::Node(Macro* _macro):
    m_macro(_macro),
    m_terminal(nullptr),
    m_cell(nullptr)
{
    nullpointer_check (_macro);

    debug && std::cout << "[Info]: New Macro Node: " 
                       << _macro->get_id() << "(" 
                       << _macro->get_name() << ")" << std::endl;
}

/**
 * @brief Constructor
 * 
 * @param _macro Node is Terminal
 */
Node::Node(Terminal* _terminal):
    m_macro(nullptr),
    m_terminal(_terminal),
    m_cell(nullptr)
{
    nullpointer_check (_terminal);

    debug && std::cout << "[Info]: New Terminal Node: " 
                       << _terminal->get_name() << std::endl;
}

/**
 * @brief Constructor
 * 
 * @param _macro Node is Cell
 */
Node::Node(Cell* _cell):
    m_macro(nullptr),
    m_terminal(nullptr),
    m_cell(_cell)
{
    nullpointer_check (_cell);

    debug && std::cout << "[Info]: New Cell Node: " 
                       << _cell->get_id() << "(" 
                       << _cell->get_name() << ")" << std::endl;
}

/**
 * @brief Destructor
 */
Node::~Node()
{
    m_macro = nullptr;
}

/**
 * @brief Check if Node is Macro or Cell
 * 
 * @return bool
 */
bool Node::is_node()
{
    return m_macro != nullptr || m_cell != nullptr;
}

/**
 * @brief Check if Node it Terminal
 * 
 * @return bool
 */
bool Node::is_terminal()
{
    return m_terminal != nullptr;
}

/**
 * @brief Check if Node is Cell
 * 
 * @return bool
 */
bool Node::has_cell()
{
    return this->is_node() && this->get_cell() != nullptr;
}

/**
 * @brief Check if Node is Macro
 * 
 * @return bool
 */
bool Node::has_macro()
{
    return this->is_node() && this->get_macro() != nullptr;
}

/**
 * @brief Get Macro
 * 
 * @return Placer::Macro*
 */
Macro* Node::get_macro()
{
    return m_macro;
}

/**
 * @brief Get Terminal
 * 
 * @return Placer::Terminal*
 */
Terminal* Node::get_terminal()
{
    return m_terminal;
}

/**
 * @brief Get Cell
 * 
 * @return Placer::Cell*
 */
Cell* Node::get_cell()
{
    return m_cell;
}

/**
 * @brief Get Edges Connected to Node
 * 
 * @return std::set< Placer::Edge* >
 */
std::set<Edge*> Node::get_edges()
{
    return m_edges;
}

/**
 * @brief Insert New Edge to Node
 * 
 * @param _edge New Edge
 */
void Node::insert_edge(Edge* _edge)
{
    nullpointer_check (_edge);

    m_edges.insert(_edge);
}

/**
 * @brief Dump Node Information
 * 
 * @param stream Stream to Dump to
 */
void Node::dump(std::ostream& stream)
{
    stream << std::string(30, '#') << std::endl;
    if(this->is_terminal()){
        m_terminal->dump(stream);
    } else if (this->has_cell()){
        m_cell->dump();
    } else if(this->has_macro()){
        m_macro->dump();
    }

    for(auto itor: m_edges){
        itor->dump(stream);
    }
    stream << std::string(30, '#') << std::endl;
}

/**
 * @brief Get Id of Element behind Node
 * 
 * @return std::string
 */
std::string Node::get_id()
{
    if(m_cell != nullptr){
        return m_cell->get_id();
    } else if (m_terminal != nullptr){
        return m_terminal->get_id();
    } else if (m_macro != nullptr){
        return m_macro->get_id();
    } else {
        notsupported_check("Only Macros, Terminals and Cells are supported!")
    }
}

/**
 * @brief Get Name of Element behind Node
 * 
 * @return std::string
 */
std::string Node::get_name()
{
     if(m_cell != nullptr){
        return m_cell->get_name();
    } else if (m_terminal != nullptr){
        return m_terminal->get_name();
    } else if (m_macro != nullptr){
        return m_macro->get_name();
    } else {
        notsupported_check("Only Macros, Terminals and Cells are supported!")
    }
}
