//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : supplement.hpp
//
// Date         : 09. January 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : LEF/DEF Supplement File
//==================================================================
#ifndef SUPPLEMENT_HPP
#define SUPPLEMENT_HPP

#include <object.hpp>
#include <logger.hpp>
#include <supplementmacro.hpp>

#include <json/json.h>
#include <map>
#include <iostream>

namespace Placer {

class Supplement: public virtual Object{
public:

    Supplement();

    ~Supplement();

    void read_supplement_file();
    
    bool has_supplement();
    
    bool has_macro(std::string const & id);
    
    SupplementMacro* get_macro(std::string const & id);

private:
    Utils::Logger* m_logger;
    std::map<std::string, SupplementMacro*> m_macros;
};

}

#endif /* SUPPLEMENT_HPP */
