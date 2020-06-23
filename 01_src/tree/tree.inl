//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : tree.inl
//
// Date         : 03. March 2020
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Macro Dependency Tree
//==================================================================
#include <tree.hpp>
#include <edge.hpp>
#include <node.hpp>

using namespace Placer;

template<typename from,typename to>
void Tree::insert_edge(from* from_val,
                       to* to_val,
                       std::string const & from_pin,
                       std::string const & to_pin,
                       std::string const & edge_name)
{
    nullpointer_check (from_val);
    nullpointer_check (to_val);

    Node* _from = this->find_node(from_val->get_name(), from_val->get_id());
    Node* _to = this->find_node(to_val->get_name(), to_val->get_id());

    if(_from == nullptr){
        _from = new Node(from_val);
        this->new_element(_from);
    }
    
    if(_to == nullptr){
        _to = new Node(to_val);
        this->new_element(_to);
    }

    Edge* _edge = new Edge(_from, _to, from_pin, to_pin, edge_name);
    m_edges.push_back(_edge);

    m_logger->insert_edge(_from->get_id(), _to->get_id());
}
