//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : base_database.hpp
//
// Date         : 15.May 2020
// Compiler     : gcc version 9.3.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Abstract Base Database Class
//==================================================================
#ifndef BASE_DATABASE_HPP
#define BASE_DATABASE_HPP

#include <sqlite3.h>
#include <string>
#include <iostream>
#include <exception>

#include <utils.hpp>

namespace Placer::Utils {

/**
 * @class BaseDatabase
 * @brief Base Class for all Database Communication
 */ 
class BaseDatabase {
public:

    virtual int __callback__(int argc, char **argv, char **azColName) = 0;
    static int c_callback(void *data, int argc, char **argv, char **azColName);

    void db_command(std::string const & query);

protected:
    BaseDatabase(std::string const & url);

    virtual ~BaseDatabase();

    void open_database();
    void close_database();

    sqlite3* p_db;
    std::string p_db_url;
    bool p_debug;
};

} /* namespace Placer::Utils */

#endif /* BASE_DATABASE_HPP */
