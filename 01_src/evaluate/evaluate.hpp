//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : encoding.hpp
//
// Date         : 27. December 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Evaluate Placement Result
//==================================================================
#ifndef EVALUATE_HPP
#define EVALUATE_HPP

#include <object.hpp>
#include <component.hpp>
#include <macrocircuit.hpp>

#include <cmath>
#include <algorithm>
#include <vector>
#include <iterator>

namespace Placer {

class MacroCircuit;

class Evaluate: public virtual Object {
public:
    Evaluate(MacroCircuit* mckt);

    ~Evaluate();

    std::pair<size_t, size_t> best_hpwl();

    std::vector<std::pair<size_t, size_t>> all_hpwl();

    std::vector<std::pair<size_t, size_t>> all_area();

    std::pair<size_t, size_t> best_area();

    void plot_hpwl_distribution();

private:
    friend class MacroCircuit;

    MacroCircuit* m_mckt;

    size_t calculate_hpwl(size_t const solution);
    size_t calculate_area(size_t const solution);

    size_t euclidean_distance(std::pair<size_t, size_t> const & from,
                              std::pair<size_t, size_t> const & to);

    size_t manhattan_distance(std::pair<size_t, size_t> const & from,
                              std::pair<size_t, size_t> const & to);
};

}

#endif /* EVALUATE_HPP */
