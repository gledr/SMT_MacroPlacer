//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : database.hpp
//
// Date         : 15.May 2020
// Compiler     : gcc version 9.3.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Database Class to Store Results
//==================================================================
#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <object.hpp>
#include <base_database.hpp>

namespace Placer::Utils {

class Database: public virtual Placer::Object,
                public virtual Placer::Utils::BaseDatabase {
public:
    Database(std::string const & db_file);

    virtual ~Database();

private:
    
};

}

#endif /* DATABASE_HPP */
