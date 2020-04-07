//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : parquet.hpp
//
// Date         : 13. March 2020
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Parquet Frontend
//==================================================================
#ifndef PARQUET_HPP
#define PARQUET_HPP

#include <fstream>
#include <boost/filesystem.hpp>

#include <layout.hpp>
#include <macro.hpp>
#include <terminal.hpp>
#include <tree.hpp>
#include <object.hpp>
#include <utils.hpp>

#include <Parquet.h>
#include <mixedpackingfromdb.h>
#include "btreeanneal.h"

namespace Placer {

/**
 * @class ParquetFrontend
 * @brief Utilize Parquet Floorplanner alternative solutions
 */
class ParquetFrontend: public virtual Object {
public:
    ParquetFrontend();

    virtual ~ParquetFrontend();

    void set_macros(std::vector<Macro*> & macros);
    void set_terminals(std::vector<Terminal*> & terminals);
    void set_tree(Tree* tree);
    void set_layout(Layout* layout);

    void build_db();

    void run_parquet();

    void data_from_parquet();

    void store_bookshelf_results();

private:

    std::vector<Macro*> m_macros;
    std::vector<Terminal*> m_terminals;
    Tree* m_tree;
    Layout* m_layout;

    parquetfp::DB* m_db;
    parquetfp::Nodes* m_nodes;
    parquetfp::Nets* m_nets;

    Macro* find_macro(std::string const & name);
};

}

#endif /* PARQUET_HPP */
