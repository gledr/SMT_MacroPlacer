//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : utils.inl
//
// Date         : 07.May 2020
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Utility Class
//==================================================================
#include <utils.hpp>

#include <vector>

/*
 * Can not use "using namespace" here due to linker failure
 */


/**
 * @brief Calculate the Greatest Common Divider of a vector
 * 
 * @param input Vector of Template Type
 * @return T
 */
template<typename T>
T Placer::Utils::Utils::gcd(std::vector<T> const & input)
{
    assert (input.size());
    
    size_t ret_val= input[0];
    
    for (size_t i =1 ; i < input.size(); ++i){
        ret_val = std::__gcd(input[i], ret_val);
    }
    
    return ret_val;
}
