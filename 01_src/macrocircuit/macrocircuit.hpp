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
#include <thread>
#include <chrono>
#include <mutex>
#include <cmath>
#include <omp.h>

#include <z3++.h>

#include <boost/filesystem.hpp>

#include <object.hpp>
#include <lefdefIO.h>
#include <macro.hpp>
#include <macro_definition.hpp>
#include <cell.hpp>
#include <terminal.hpp>
#include <tree.hpp>
#include <evaluate.hpp>
#include <logger.hpp>
#include <timer.hpp>
#include <supplement.hpp>
#include <bookshelf.hpp>
#include <layout.hpp>
#include <partition.hpp>
#include <partitioning.hpp>
#include <parquet.hpp>
#include <encoding_utils.hpp>
#include <utils.hpp>
#include <costfunction.hpp>

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

    size_t get_solutions();

    void build_circuit();
    void partitioning();
    void encode();
    void place();

    Circuit::Circuit* get_circuit();

    void dump_all();
    void dump_best();

    void save_all();
    void save_best();

    void best_result();

    void dump(std::ostream & stream = std::cout);

    void store_results();

    std::vector<Macro*>& get_macros();
    std::vector<Partition*>& get_partitions();
    std::vector<Terminal*> get_terminals();
    std::vector<Component*> get_components();

    Tree* get_tree();
    Layout* get_layout();

    size_t get_minimal_die_size_prediction();

    std::pair<size_t, size_t> biggest_macro();

private:
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
    Supplement* m_supplement;
    Bookshelf* m_bookshelf;
    Partitioning* m_partitioning;
    ParquetFrontend* m_parquet;
    EncodingUtils* m_encode;
    CostFunction* m_lut;

    std::map<std::string, Macro*> m_id2macro;
    std::map<std::string, Terminal*> m_id2terminal;
    std::map<std::string, Cell*> m_id2cell;
    size_t m_solutions;
    size_t m_estimated_area;
    
    size_t m_layout_x;
    size_t m_layout_y;

    size_t m_gcd_h;
    size_t m_gcd_w;
    
    Layout* m_layout;

    double m_standard_cell_height;

    bool is_macro(LefDefParser::defiComponent const & macro);
    bool is_standard_cell(LefDefParser::defiComponent const & cell);

    /**
     * Thread Functions
     */
    void add_terminals();
    void add_macros();
    void add_cells();
    void area_estimator();
    void grid_builder(size_t const start_point);
    std::mutex mtx;

    void add_cell(LefDefParser::defiComponent const & cmp);

    void build_tree();
    void create_macro_definitions();
    void calculate_gcd();
    
    void create_image(size_t const solution);
    void write_def(std::string const & name, size_t const solution);

    /**
     * SMT Encoding
     */
    void encode_smt();
    void encode_parquet();
    void encode_grid();

    void config_z3();
    void run_encoding();

    void encode_components_inside_die(eRotation const type);
    void encode_components_non_overlapping(eRotation const type);
    void encode_terminals_on_frontier();
    void encode_terminals_non_overlapping();

    z3::expr m_components_non_overlapping;
    z3::expr m_components_inside_die;
    z3::expr m_terminals_on_frontier;
    z3::expr m_terminals_non_overlapping;

    /**
     * SMT Solving
     */ 
    void solve();
    void dump_smt_instance();
};

}

#endif /* MACROCIRCUIT_HPP */
