//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : base_database.cpp
//
// Date         : 15.May 2020
// Compiler     : gcc version 9.3.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Abstract Base Database Class
//==================================================================
#include "base_database.hpp"

using namespace Placer::Utils;

/**
 * @brief Constructor
 * 
 * @param url: Location of the database file
 */
BaseDatabase::BaseDatabase(std::string const & url)
{
    p_db_url = url;
    p_debug = false;
}

/**
 * @brief Destructor
 */
BaseDatabase::~BaseDatabase()
{
}

////////////////////////////////////////////////////////////////////////////
//                  BaseDatabase Basic Communication Functions
////////////////////////////////////////////////////////////////////////////
/**
 * @brief Execute an SQL query
 * @param command The SQL query to be executed
 */
void BaseDatabase::db_command(std::string const & command)
{
    this->open_database();
    p_debug && std::cout << Utils::Utils::get_bash_string_orange("db_command: " + command) << std::endl;
   
    char const * str = command.c_str();
    if(sqlite3_exec(p_db, str, c_callback, this, NULL) != 0){
        throw std::runtime_error(std::string(sqlite3_errmsg(p_db)));
    }
    this->close_database();
}

/**
 * @brief Opens the BaseDatabase for a transaction
 */
void BaseDatabase::open_database()
{
    p_debug && std::cout<< "Open BaseDatabase Connection (" << p_db_url << ")" << std::endl;
    if((sqlite3_open(p_db_url.c_str(), &p_db)) != SQLITE_OK){
        throw std::runtime_error(std::string(sqlite3_errmsg(p_db)));
    }
    p_debug && std::cout << "BaseDatabase Connection Established!" << std::endl;
}

/**
 * @brief Close the BaseDatabase after a transation has been performed
 */ 
void BaseDatabase::close_database()
{
    p_debug && std::cout << "Close BaseDatabase has been called" << std::endl;
    if(p_db){
        sqlite3_close(p_db);
    }
}

/**
 * @brief Static Callback function for the Sqlite3 API
 * This function calls a member function in order to get access to the
 * this pointer and performs the operations in this function finally
 */
int BaseDatabase::c_callback(void *data, int argc, char **argv, char **azColName)
{
    BaseDatabase* tmp = reinterpret_cast<BaseDatabase*>(data);
    return tmp->__callback__(argc, argv, azColName);
}
