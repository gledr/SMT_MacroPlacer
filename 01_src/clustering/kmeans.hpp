//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : kmeans.hpp
//
// Date         : 19. March 2020
// Compiler     : gcc version 9.2.1 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : KMeans Clustering
//==================================================================
#ifndef KMEANS_HPP
#define KMEANS_HPP

#include <object.hpp>
#include <centroid.hpp>
#include <cluster.hpp>

#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/generator_iterator.hpp>

namespace Placer {

/**
 * @class KMeans
 * 
 * @brief Perform KMeans Clustering for Macros
 */
class KMeans: public virtual Object
{
public:

    KMeans();

    virtual ~KMeans();

    void set_problem(std::vector<Component*> const & components,
                     size_t const cluster);

    void initialize();

    void run(size_t const iterations);

    void dump(std::ostream & stream = std::cout);

    void plot();

    std::vector<Cluster*> get_cluster();

private:
    
    void generate_initial_centroid_positions();
    void map_macros_to_cluster();

    std::vector<Component*> m_components;
    std::vector<Cluster*> m_cluster;
    size_t m_number_of_cluster;
    size_t m_iterations;

    size_t m_max_height;
    size_t m_max_width;
};

} /* namespace Placer */

#endif /* KMEANS_HPP */
