//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : centroid.hpp
//
// Date         : 19. March 2020
// Compiler     : gcc version 9.2.1 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Clustering Centroid
//==================================================================
#ifndef CENTROID_HPP
#define CENTROID_HPP

#include <object.hpp>
#include <partition.hpp>
#include <component.hpp>

namespace Placer {

/**
 * @class Centroid
 * 
 * @brief Centroid Node for Clustering
 */
class Centroid: public virtual Object {
public:
    Centroid (size_t const x,
              size_t const y);
    
    virtual ~Centroid();
    
    void update_centroid(size_t const x,
                         size_t const y);

    double eucledian_distance(Component* c);

    void dump(std::ostream & stream = std::cout);
    
    size_t get_height();
    size_t get_width();

private:
    size_t m_width;
    size_t m_height;

    std::vector<size_t> m_width_history;
    std::vector<size_t> m_height_history;
};

} /* namespace Placer */

#endif /* CENTROID_HPP */
