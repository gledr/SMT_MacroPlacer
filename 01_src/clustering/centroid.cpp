//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : centroid.cpp
//
// Date         : 19. March 2020
// Compiler     : gcc version 9.2.1 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Clustering Centroid
//==================================================================
#include "centroid.hpp"

using namespace Placer;

/**
 * @brief Constructor
 * 
 * @param pos_x Initial X Position
 * @param pos_y Initial Y Position
 */
Centroid::Centroid(size_t const pos_x,
                   size_t const pos_y)
{
    m_width = pos_x;
    m_height = pos_y;
}

/**
 * @brief Destructor
 */
Centroid::~Centroid()
{
}

/**
 * @brief Update Centroid Position
 * 
 * @param x X Position
 * @param y Y Position
 */
void Centroid::update_centroid(size_t const x, size_t const y)
{
    m_width_history.push_back(m_width);
    m_height_history.push_back(m_height);
    
    m_width = x;
    m_height = y;
}

/**
 * @brief Calculate the eucledian distance to the macro
 * 
 * @param m Pointer to the macro
 * @return double
 */
double Centroid::eucledian_distance(Component* c)
{
    nullpointer_check (c);
    
    int mw = c->get_width().get_numeral_uint();
    int mh = c->get_width().get_numeral_uint();
    
    int w = m_width - mw;
    int h = m_height - mh;
    
    int ww = pow(w, 2);
    int hh = pow(h, 2);
 
    return sqrt(ww + hh);
}

/**
 * @brief Dump Centroid Information
 */
void Centroid::dump(std::ostream & stream)
{
    stream << "Centroid: " << m_width << " x " << m_height << std::endl;
}

/**
 * @brief Get acutal height/Y Position of the Centroid
 * 
 * @return size_t
 */
size_t Centroid::get_height()
{
    return m_height;
}

/**
 * @brief Get actual width/X Position of the Centroid
 * 
 * @return size_t
 */
size_t Centroid::get_width()
{
    return m_width;
}
