//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : database.cpp
//
// Date         : 15.May 2020
// Compiler     : gcc version 9.3.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Database Class to Store Results
//==================================================================
#include "database.hpp"

using namespace Placer::Utils;

Database::Database(std::string const & db_file):
    Object(),
    BaseDatabase(db_file)
{
}

Database::~Database()
{
}

