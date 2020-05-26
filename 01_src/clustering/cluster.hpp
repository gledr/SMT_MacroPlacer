//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : cluster.hpp
//
// Date         : 19. March 2020
// Compiler     : gcc version 9.2.1 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Cluster for Partitioning
//==================================================================
#ifndef CLUSTER_HPP
#define CLUSTER_HPP

#include <object.hpp>
#include <centroid.hpp>

namespace Placer {

/**
 * @class Cluster
 * 
 * @brief Single Cluster for KMeans Clustering
 */
class Cluster: public virtual Object {
public:
    Cluster(std::string const & id,
            size_t const x,
            size_t const y,
            size_t const max_size);

    virtual ~Cluster();

    void insert_partition(Partition* p);

    double eucledian_distance(Component* c);

    std::vector<Partition*> get_partitions();

    bool has_space();

    std::string get_id();

    double get_sum_of_distances();

    size_t size();

    void recenter();

    void reset();

    void dump(std::ostream & stream = std::cout);

    std::pair<size_t, size_t> get_centroid_position();

private:
    Centroid* m_centroid;
    size_t m_max_size;
    std::string m_id;

    std::vector<Partition*> m_partitons;
};

} /* namespace Placer */

#endif /* CLUSTER_HPP */
