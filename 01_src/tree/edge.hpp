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
#include <cassert>
#include <string>

namespace Placer {

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

    Node* get_to();
    Node* get_from();

private:
    Node* m_to;
    Node* m_from;
    std::string m_to_pin;
    std::string m_from_pin;

    std::string m_name;
};

}

#endif /* EDGE_HPP */
