//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : partitioning.hpp
//
// Date         : 24. February 2020
// Compiler     : gcc version 9.2.1 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Floorplan Partitioning
//==================================================================
#ifndef PARTITIONING_HPP
#define PARTITIONING_HPP

#include <z3++.h>
#include <libkahypar.h>
#include <cmath>
#include <memory>

#include <object.hpp>
#include <partition.hpp>
#include <macro.hpp>
#include <terminal.hpp>
#include <encoding_utils.hpp>
#include <cluster.hpp>
#include <kmeans.hpp>
#include <exception.hpp>
#include <tree.hpp>

namespace Placer {

/**
 * @class Partitioning
 * 
 * @brief Split Problem into Subproblems 
 */
class Partitioning: public virtual Object {
public:
    Partitioning();

    virtual ~Partitioning();

    void set_problem(std::vector<Macro*> & macros,
                     std::vector<Terminal*> & terminals,
                     Tree* tree);

    void run();

    void hypergraph_partitioning();

    void kmeans_clustering();

    std::vector<Component*> get_partitions();

private:
    z3::optimize* m_z3_opt;
    EncodingUtils* m_encode;
    KMeans* m_kmeans;
    Tree* m_tree;
    Utils::Logger* m_logger;

    std::vector<Component*> m_components;
    std::vector<Macro*> m_macros;
    std::vector<Terminal*> m_terminals;

    void create_initial_partitions();
    Macro* find_macro(std::string const & id);

    /**
     * SMT Encoding
     */
    void encode(Partition* next_partition);
    void encode_components_in_partition(Partition* next_partition,
                                        eRotation const type);

    void encode_components_non_overlapping(Partition* next_partition,
                                          eRotation const type);

    void encode_hpwl_cost_function(Partition* next_partition);

    z3::expr m_components_in_partition;
    z3::expr m_components_non_overlapping;
    z3::expr m_hpwl_cost_function;

    /**
     * SMT Solver
     */
    void solve(Partition* next_partition);
    std::pair<size_t, size_t> find_shape(size_t const area);
    
    /**
     * Hypergraph Partitioning
     */
    void file_based_partitioning();
    void api_based_partitioning();
};

} /* namespace Placer */

#endif /* PARTITIONING_HPP */
