//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : edge.hpp
//
// Date         : 08. January 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Macro Dependency Tree Edge
//==================================================================
#ifndef EDGE_HPP
#define EDGE_HPP

#include <iostream>
#include <string>
#include <exception.hpp>

namespace Placer {

enum eEdgeType {
    eSignal       = 0,
    ePower        = 1,
    eUnknownEdge  = 2};
    
class Node;

/** 
 * @class Edge
 * 
 * @brief Tree Connection Edge between Nodes
 */
class Edge {
public:

    Edge(Node* from,
         Node* to,
         std::string const & from_pin,
         std::string const & to_pin,
         std::string const & edge_name);

    virtual ~Edge();

    std::string get_name();

    void dump(std::ostream & stream = std::cout);

    bool is_terminal_to_cell();
    bool is_terminal_to_macro();
    bool is_macro_to_macro();
    bool is_macro_to_cell();
    bool is_cell_to_macro();
    bool is_cell_to_terminal();
    bool is_macro_to_terminal();

    std::string get_from_pin() const;
    std::string get_to_pin() const;

    Node* get_to();
    Node* get_from();

    size_t get_frequency_from();
    size_t get_frequency_to();

    size_t get_bitwidth_from();
    size_t get_bitwidth_to();

    bool is_power_edge();
    bool is_signal_edge();

private:
    Node* m_to;
    Node* m_from;
    std::string m_to_pin;
    std::string m_from_pin;
    eEdgeType m_edge_type;
    std::string m_name;

    void resolve_edge_type();
};

} /* namespace Placer */

#endif /* EDGE_HPP */
