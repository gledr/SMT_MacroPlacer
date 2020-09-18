//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : macrocircuit.hpp
//
// Date         : 22. December 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : SoC Macro Circuit
//==================================================================
#ifndef MACROCIRCUIT_HPP
#define MACROCIRCUIT_HPP

#include <vector>
#include <string>
#include <fstream>
#include <cstdio>
#include <map>
#include <unordered_map>
#include <thread>
#include <chrono>
#include <mutex>
#include <cmath>

#include <z3++.h>

#include <boost/filesystem.hpp>

#include <object.hpp>
#include <components.hpp>
#include <lefdefIO.h>
#include <tree.hpp>
#include <evaluate.hpp>
#include <logger.hpp>
#include <timer.hpp>
#include <supplement.hpp>
#include <bookshelf.hpp>
#include <layout.hpp>
#include <partitioning.hpp>
#include <parquet.hpp>
#include <encoding_utils.hpp>
#include <utils.hpp>
#include <def_utils.hpp>
#include <database.hpp>
#include <plotter.hpp>
#include <hlclient.hpp>

namespace Placer {

class Evaluate;
class Partitioning;
class ParquetFrontend;

/**
 * @class MacroCircuit
 * @brief Top Class for MacroCircuit Placement
 */
class MacroCircuit: public virtual Object {
public:

    MacroCircuit();

    virtual ~MacroCircuit();

    void build_circuit();
    void partitioning();
    void encode();
    void place();

    void dump_all();
    void dump_best();

    void save_all();
    void save_best();

    void results_to_db();

    void create_statistics();

    void dump(std::ostream & stream = std::cout);

private:
    friend class Evaluate;

    // Microsoft Z3
    z3::optimize* m_z3_opt;

    Circuit::Circuit* m_circuit;
    std::vector<Macro*> m_macros;
    std::vector<MacroDefinition> m_macro_definitions;

    std::vector<Cell*> m_cells;

    std::vector<Terminal*> m_terminals;

    std::vector<Partition*> m_partitons;

    std::vector<Component*> m_components;
    Tree* m_tree;
    Evaluate* m_eval;
    Utils::Logger* m_logger;
    Utils::Timer* m_timer;
    Utils::Database* m_db;
    Utils::DefUtils* m_def_utils;
    Supplement* m_supplement;
    Bookshelf* m_bookshelf;
    Partitioning* m_partitioning;
    ParquetFrontend* m_parquet;
    EncodingUtils* m_encode;
    Plotter* m_plotter;
    HLClient* m_hl_client;

    std::map<std::string, Macro*> m_id2macro;
    std::map<std::string, Terminal*> m_id2terminal;
    std::map<std::string, Cell*> m_id2cell;
    size_t m_solutions;
    size_t m_estimated_area;

    Layout* m_layout;

    double m_standard_cell_height;

    void build_circuit_lefdef();
    void build_circuit_bookshelf();

    bool is_macro(LefDefParser::defiComponent const & macro);
    bool is_standard_cell(LefDefParser::defiComponent const & cell);

    /**
     * Thread Functions
     */
    void add_terminals();
    void add_macros();
    void add_cells();
    void area_estimator();

    void add_cell(LefDefParser::defiComponent const & cmp);

    void build_tree_from_lefdef();
    void init_tree(eInputFormat const type);

    void create_macro_definitions();

    void write_def(std::string const & name, size_t const solution);
    void write_lef(std::string const & name);

    Tree* get_tree();
    Layout* get_layout();
    size_t get_solutions();

    /**
     * SMT Encoding
     */
    void encode_smt();
    void encode_parquet();

    void config_z3();
    void run_encoding();

    void encode_components_inside_die(eRotation const type);
    void encode_components_non_overlapping(eRotation const type);
    void encode_terminals_on_frontier();
    void encode_terminals_non_overlapping();
    void encode_terminals_center_edge();
    void encode_hpwl_length();
    void encode_layout_on_grid();
    void encode_components_on_grid();
    void encode_terminals_on_grid();

    z3::expr m_components_non_overlapping;
    z3::expr m_components_inside_die;
    z3::expr m_terminals_on_frontier;
    z3::expr m_terminals_non_overlapping;
    z3::expr m_terminals_center_edge;
    z3::expr m_hpwl_cost_function;
    z3::expr m_layout_on_grid;
    z3::expr m_terminals_on_grid;
    z3::expr m_components_on_grid;
    z3::expr_vector m_hpwl_edges;

    z3::expr manhattan_distance(z3::expr const & from_x,
                                z3::expr const & from_y,
                                z3::expr const & to_x,
                                z3::expr const & to_y);

    /**
     * SMT Solving
     */ 
    void solve_z3_api();
    void solve_z3_no_api();
    void solve_optimathsat_no_api();
    void process_results(z3::model const & m);
    void dump_smt_instance();
    void process_key_value_results(std::map<std::string, std::vector<size_t>> & solution, size_t const id);

};

} /* namespace Placer */

#endif /* MACROCIRCUIT_HPP */
