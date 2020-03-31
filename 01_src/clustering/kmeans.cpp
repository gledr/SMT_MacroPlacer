//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : kmeans.cpp
//
// Date         : 19. March 2020
// Compiler     : gcc version 9.2.1 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : KMeans Clustering
//==================================================================
#include "kmeans.hpp"

using namespace Placer;

/**
 * @brief Constructor
 */
KMeans::KMeans()
{
}

/**
 * @brief Destructor
 */
KMeans::~KMeans()
{
    for (auto itor: m_cluster){
        delete itor; itor = nullptr;
    }
}

/**
 * @brief Set the Problem for KMeans
 * 
 * @param macros  The Macros to Perform KMeans on
 * @param cluster The Number of Clusters
 */
void KMeans::set_problem(std::vector<Component *> const & components,
                         size_t const cluster)
{
    m_components = components;
    m_number_of_cluster = cluster;
}

/**
 * @brief Set Random Positions for Centroid at the beginning 
 */
void KMeans::generate_initial_centroid_positions()
{
    boost::minstd_rand generator(42);
    boost::uniform_int<> uni_dist_height(0,m_max_height);
    boost::uniform_int<> uni_dist_width(0,m_max_width);
    
    boost::variate_generator<boost::minstd_rand&, boost::uniform_int<> > uni_height(generator, uni_dist_height);
    boost::variate_generator<boost::minstd_rand&, boost::uniform_int<> > uni_width(generator, uni_dist_height);
    
    for (size_t i = 0; i < m_number_of_cluster; ++i){
        size_t x = uni_width();
        size_t y = uni_height();
        std::string id = "cluster" + std::to_string(i);
        
        Cluster* new_cluster = new Cluster(id, x, y, 3);
        m_cluster.push_back(new_cluster);
    }
    
    std::cout << "Initial Centroid Positions: " << std::endl;
    for (auto itor: m_cluster){
        std::cout << itor->get_id() << " " << itor->get_centroid_position().first << " " << itor->get_centroid_position().second << std::endl;
    }
    std::cout << "--------------" << std::endl;
}

/**
 * @brief Initialize KMeans
 */
void KMeans::initialize()
{
    m_max_height = 0;
    m_max_width = 0;
    
    for (Component* c : m_components){
        if (c->get_width().get_numeral_uint() > m_max_width){
            m_max_width = c->get_width().get_numeral_uint();
        }
        if (c->get_height().get_numeral_uint() > m_max_height){
            m_max_height = c->get_height().get_numeral_uint();
        }
    }
    
    this->generate_initial_centroid_positions();
}

/**
 * @brief Map the Macro to the Closest Cluster
 */
void KMeans::map_macros_to_cluster()
{
    for (auto partition: m_components){
        std::vector<std::pair<Cluster*, double>> distances;
        for (auto cluster: m_cluster){
            double distance = cluster->eucledian_distance(partition);
            distances.push_back(std::make_pair(cluster, distance));
        }
        
        std::sort(distances.begin(), distances.end(), [](std::pair<Cluster*, double> a, std::pair<Cluster*, double> b){
                if (a.second < b.second){
                    return true;
                }  else {
                    return false;
                }
            }
        );
        bool done = false;
        for (size_t i = 0; i < distances.size(); ++i){
            if (distances[i].first->has_space()){
                distances[i].first->insert_partition(dynamic_cast<Partition*>(partition));
                done = true;
                break;
            }
        }
        assert (done);
    }
}

/**
 * @brief Run KMeans Clustering
 * 
 * @param iterations
 */
void KMeans::run(size_t const iterations)
{
    m_iterations = iterations;
   
    for (size_t i = 0; i < m_iterations; ++i){
        std::cout << "KMeans Iteration..." << std::endl;
        for (auto cluster: m_cluster){
            cluster->reset();
        }
        
        this->map_macros_to_cluster();
        
        for (auto cluster: m_cluster){
            cluster->recenter();
        }
    }
    std::cout << "KMeans done" << std::endl;
    this->plot();
}

/**
 * @brief Dump KMeans Information
 */
void KMeans::dump(std::ostream & stream)
{
    stream << "========================" << std::endl;
    for (auto itor: m_cluster){
        itor->dump(stream);
    }
    stream << "========================" << std::endl;
}

/**
 * @brief Get Final Cluster
 * 
 * @return std::vector< Placer::Cluster* >
 */
std::vector<Cluster*> KMeans::get_cluster()
{
    return m_cluster;
}

/**
 * @brief Plot KMeans Results using GnuPlot
 */
void KMeans::plot()
{
    if (!boost::filesystem::exists(this->get_image_directory())){
        boost::filesystem::create_directories(this->get_image_directory());
    }
    
    boost::filesystem::current_path(this->get_image_directory());
    
    std::ofstream macros ("macros.txt");
    for (auto itor: m_components){
        macros << itor->get_width() << " " << itor->get_height() << std::endl;
    }
    macros.close();
    
    std::ofstream centroids("centroids.txt");
    for (auto itor: m_cluster){
        auto _centroid_pos = itor->get_centroid_position();
        centroids << _centroid_pos.first << " " << _centroid_pos.second << std::endl;
    }
    centroids.close();
}
