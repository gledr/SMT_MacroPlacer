//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : timer.cpp
//
// Date         : 18.August 2020
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Timer Class
//==================================================================
#include "timer.hpp"

using namespace Placer::Utils;


/**
 * @brief Constructor
 */
Timer::Timer()
{
}

/**
 * @brief Destructor
 */
Timer::~Timer()
{
}

/**
 * @brief Start Timer Instance for given ID
 * 
 * @param id ID
 */
void Timer::start_timer(std::string const & id)
{
    m_begin[id] = std::chrono::system_clock::now();
}

/**
 * @brief Stop Timer Instance for given ID
 * 
 * @param id ID
 */
void Timer::stop_timer(std::string const & id)
{
    m_end[id] = std::chrono::system_clock::now();
}

/**
 * @brief Read Timer Result for given ID in milliseconds
 *
 * @param id ID
 * @return size_t
 */
size_t Timer::read_timer_ms(std::string const & id)
{
    auto start = m_begin[id];
    auto end   = m_end[id];
    auto duration = end -start;
    
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

/**
 * @brief Read Timer Result for given ID in seconds
 *
 * @param id ID
 * @return size_t
 */
size_t Timer::read_timer_sec(std::string const & id)
{
    auto start = m_begin[id];
    auto end   = m_end[id];
    auto duration = end - start;
    size_t val = std::chrono::duration_cast<std::chrono::seconds>(duration).count();

    return val;
}
