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
#include <macro.hpp>
#include <terminal.hpp>
#include <logger.hpp>

namespace Placer::Utils {

class Database: public virtual Placer::Object,
                public virtual Placer::Utils::BaseDatabase {
public:
    Database(std::string const & db_file);

    virtual ~Database();

    virtual int __callback__(int argc, char **argv, char **azColName);

    void init_database();

    void place_component(size_t const solution, Component* component);

    void place_pin(size_t const solution, Component* parent, Pin* pin);

    void place_terminal(size_t const solution, Terminal* terminal);

    void insert_results(size_t const solution,
                        size_t const area,
                        size_t const hpwl);

    void insert_layout(size_t const solution,
                       size_t const lx,
                       size_t const ly,
                       size_t const ux,
                       size_t const uy);

    void export_as_csv(std::string const & filename);

private:
    Database (Database const & db);
    Database operator= (Database const & db);
    bool operator== (Database const & db);

    Logger* m_logger;

    enum db_transaction {
        e_init                      = 0  ///<
    };

    db_transaction p_active_transaction;
};

} /* namespace Placer::Utils */

#endif /* DATABASE_HPP */
