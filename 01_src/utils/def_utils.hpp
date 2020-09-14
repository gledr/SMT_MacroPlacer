//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : def_utils.hpp
//
// Date         : 24.August 2020
// Compiler     : gcc version 10.0.2 (GCC)
// Copyright    : Johannes Kepler University
// Description  : Utilities for DEF Handling
//==================================================================
#ifndef DEF_UTILS_HPP
#define DEF_UTILS_HPP

#include <string>
#include <exception.hpp>
#include <object.hpp>

namespace Placer::Utils {

class DefUtils : public virtual Object {
public:
    /**
     * @brief Calculate Def Units to Microns
     * 
     * @param val Def Value
     * @return size_t
     */
    size_t def_to_microns(size_t const val)
    {
        return val * this->get_def_units();
    }

    /**
     * @brief Calculate Micons to Def Units
     * 
     * @param val Microns Value
     * @return size_t
     */
    size_t microns_to_def(size_t const val)
    {
        if ((val % this->get_def_units()) != 0){
            std::cout << "Value: " << val << std::endl;
            std::cout << "Unit: " << this->get_def_units() << std::endl;
            throw PlacerException ("DEF Units can not be processed without information loss! \nPlease Check Def Units!");
        }
        return val / this->get_def_units();
    }
};

} /* namespace Placer */ 

#endif /* DEF_UTILS_HPP */
