//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : cluster.cpp
//
// Date         : 19. March 2020
// Compiler     : gcc version 9.2.1 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Cluster for Partitioning
//==================================================================
#include "cluster.hpp"

using namespace Placer;

/**
 * @brief Constructor
 * 
 * @param id Cluster ID
 * @param x  Initial X Position of Centroid
 * @param y  Initial Y Position of Centroid
 * @param max_size Maximum Entries for Cluster
 */
Cluster::Cluster(std::string const & id,
                 size_t const x,
                 size_t const y,
                 size_t const max_size)
{
    m_id = id;
    m_centroid = new Centroid(x,y);
    m_max_size = max_size;
}

/**
 * @brief Destructor
 */
Cluster::~Cluster()
{
    delete m_centroid; m_centroid = nullptr;
}

/**
 * @brief Calculate Eucledian Distance to Centroid
 * 
 * @param m Pointer to Macro
 * @return double
 */
double Cluster::eucledian_distance(Component* c)
{
    assert (c != nullptr);
    
    return m_centroid->eucledian_distance(c);
}

/**
 * @brief Add Macro to Cluster
 * 
 * @param m Pointer to Macro
 */
void Cluster::insert_partition(Partition* p)
{
    assert (p != nullptr);
    
    m_partitons.push_back(p);
}

/**
 * @brief Calculate the new center for the given Macros
 */
void Cluster::recenter()
{
    size_t sum_height = 0;
    size_t sum_width = 0;
    
    for (Partition* p: m_partitons){
        sum_height += p->get_height().get_numeral_uint();
        sum_width  += p->get_width().get_numeral_uint();
    }
    
    size_t next_width = sum_width / m_partitons.size();
    size_t next_height = sum_height / m_partitons.size();
    
    m_centroid->update_centroid(next_width, next_height);
}

/**
 * @brief Reset Cluster and Remove Macros
 */
void Cluster::reset()
{
    m_partitons.clear();
}

/**
 * @brief Dump Cluster Information
 * 
 * @param stream Stream to dump to
 */
void Cluster::dump(std::ostream & stream)
{
    m_centroid->dump(stream);
    stream << "Partitions: " << m_partitons.size() << std::endl;
    for (auto itor: m_partitons){
        stream << itor->get_id() << ": " << itor->get_width() << "x" << itor->get_height() << std::endl;
    }
}

/**
 * @brief Get Current Cluster Size
 * 
 * @return size_t
 */
size_t Cluster::size()
{
    return m_partitons.size();
}

/**
 * @brief Get Macros assigned to Cluster
 * 
 * @return std::vector< Placer::Macro* >
 */
std::vector<Partition*> Cluster::get_partitions()
{
    return m_partitons;
}

/**
 * @brief Get Sum of Distances from Macro to Centroid
 * 
 * @return double
 */
double Cluster::get_sum_of_distances()
{
    double ret_val = 0.0;
    
    for (auto itor: m_partitons){
        ret_val += m_centroid->eucledian_distance(itor);
    }
    return ret_val;
}

/**
 * @brief Get Cluster ID
 * 
 * @return std::string
 */
std::string Cluster::get_id()
{
    return m_id;
}

/**
 * @brief Get Position of Centroid
 * 
 * @return std::pair< size_t, size_t >
 */
std::pair<size_t, size_t> Cluster::get_centroid_position()
{
    return std::make_pair(m_centroid->get_width(), m_centroid->get_height());
}

/**
 * @brief Check if Cluster accepts another macros
 * 
 * @return int
 */
bool Cluster::has_space()
{
    return m_partitons.size() < m_max_size;
}
