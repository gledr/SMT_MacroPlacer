//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : node.hpp
//
// Date         : 27. December 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Macro Dependency Tree Node
//==================================================================
#ifndef NODE_HPP
#define NODE_HPP

#include <vector>
#include <cassert>
#include <iostream>
#include <set>

#include <macro.hpp>
#include <edge.hpp>
#include <cell.hpp>
#include <terminal.hpp>
#include <exception.hpp>

namespace Placer {

/**
 * @class Node
 * 
 * @brief Connectivity Tree Node
 */ 
class Node {
public:

    Node(Macro* _macro);
    Node(Terminal* m_terminal);
    Node(Cell* m_cell);

    virtual ~Node();

    bool is_terminal();
    bool is_node();

    bool has_cell();
    bool has_macro();

    Terminal* get_terminal();
    Macro* get_macro();
    Cell* get_cell();

    std::string get_id();
    std::string get_name();

    void insert_edge(Edge* _edge);
    std::set<Edge*> get_edges();

    void dump(std::ostream & stream = std::cout);

private:
    std::set<Edge*> m_edges;
    Macro* m_macro;
    Terminal* m_terminal;
    Cell* m_cell;
};

}

#endif /* NODE_HPP */
