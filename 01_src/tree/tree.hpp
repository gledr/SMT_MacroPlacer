//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : tree.hpp
//
// Date         : 27. December 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Macro Dependency Tree
//==================================================================
#ifndef TREE_HPP
#define TREE_HPP

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <z3++.h>

#include <object.hpp>
#include <node.hpp>
#include <edge.hpp>
#include <components.hpp>
#include <logger.hpp>
#include <exception.hpp>

namespace Placer {

class Node;
class Edge;

class Tree: public virtual Object {
public:

    Tree();

    virtual ~Tree();

    Tree(Tree const & tree);

    void destroy();

    bool operator== (Tree const & tree);
    Tree& operator= (Tree const & tree);

    template<typename from,typename to>
    void insert_edge(from* from_val,
                     to* to_val,
                     std::string const & from_pin,
                     std::string const & to_pin,
                     std::string const & edge_name);

    void verify_edges();

    void construct_tree();

    void dump(std::ostream & stream = std::cout);

    std::vector<Edge*> get_edges();

    void strip_terminals();

    void visualize();

    void export_hypergraph();

    std::map<std::string, std::set<std::string>> get_steiner_tree();

private:
    Utils::Logger* m_logger;

    std::vector<Node*> m_terminals;
    std::vector<Edge*> m_edges;
    std::vector<Node*> m_nodes;
    std::map<std::string, std::set<std::string>> m_steiner_tree;

    Node* find_node(std::string const & name,
                    std::string const & id);
    void new_element(Node* _node);

    void export_dot_file();
    void dot_to_png();
    void show_png();

    void iterate_tree(std::ofstream & stream);
    void iterate_tree_recursive(Node* _root,
                                Node* current_root,
                                std::ofstream & stream,
                                std::vector<Node*> & fringe);

};

} /* namespace Placer */

#include <tree.inl>

#endif /* TREE_HPP */
