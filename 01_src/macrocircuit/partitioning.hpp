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
#include <cmath>

#include <object.hpp>
#include <partition.hpp>
#include <macro.hpp>
#include <encoding_utils.hpp>
#include <cluster.hpp>

#include <kmeans.hpp>

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
                     size_t partion_size);

    void run();
    
    void kmeans_clustering();

    std::vector<Component*> get_partitions();

private:
    z3::optimize* m_z3_opt;
    EncodingUtils* m_encode;
    KMeans* m_kmeans;
    
    std::vector<Component*> m_components;
    std::vector<Macro*> m_macros;
    size_t m_partition_size;
    
    void create_initial_partitions();
    
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
};

}

#endif /* PARTITIONING_HPP */
