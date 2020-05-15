//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : object.hpp
//
// Date         : 07.January 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Virtual Base Class
//==================================================================
#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <vector>
#include <string>
#include <cassert>

#include <z3++.h>

namespace Placer {

enum eEncodingProblem { eMacroCircuit, ePartition, eInit };
enum eRotation { eFixed, e2D, e4D };
enum eLogic { eInt, eBitVector };

double const layout_factor = 1.2;

enum eOrientation {
     eNorth     = 0,
     eWest      = 1,
     eSouth     = 2,
     eEast      = 3,
     eFlipNorth = 4,
     eFlipWest  = 5,
     eFlipSouth = 6,
     eFlipEast  = 7
};

/**
 * @class Object
 * 
 * @brief Static Superclass for configuration settings
 */
class Object {
public:
    virtual ~Object();
protected:
    Object();

    void add_lef(std::string const & lef);
    std::vector<std::string>& get_lef() const;

    void add_def(std::string const & def);
    std::string& get_def() const;

    void set_site(std::string const & site);
    std::string& get_site() const;

    void set_supplement(std::string const & file);
    std::string& get_supplement() const;

    void set_working_directory(std::string const & dir);
    std::string& get_working_directory() const;

    void set_results_directory(std::string const & name);
    std::string& get_results_directory() const;

    void set_image_directory(std::string const & name);
    std::string& get_image_directory() const;

    void set_smt_directory(std::string const & name);
    std::string get_smt_directory() const;

    void set_verbose(bool const val);
    bool get_verbose() const;

    void set_log_active(bool const val);
    bool get_log_active() const;

    void set_log_name(std::string const & log_name);
    std::string get_log_name() const;
    std::string get_log_directory();

    void set_parquet_directory(std::string const & directory);
    std::string get_parquet_directory() const;

    void set_bookshelf_file(std::string const & file);
    std::string get_bookshelf_file() const;

    void set_timeout(size_t const val);
    size_t get_timeout() const;

    void set_minimize_die_mode(bool const val);
    bool get_minimize_die_mode() const;

    void set_allsat(bool const val);
    bool get_allsat() const;

    void set_max_solutions(size_t const val);
    size_t get_max_solutions() const;

    void set_smt_to_filesystem(bool const val);
    bool get_smt_to_filesystem() const;

    void set_dump_all(bool const val);
    bool get_dump_all() const;

    void set_save_all(bool const val);
    bool get_save_all() const;

    void set_dump_best(bool const val);
    bool get_dump_best() const;

    void set_save_best(bool const val);
    bool get_save_best() const;

    void set_store_smt(bool const val);
    bool get_store_smt() const;

    void set_design_name(std::string const & name);
    std::string get_design_name() const;

    void set_binary_name(std::string const & name);
    std::string get_binary_name() const;

    void set_bitwidth_orientation(size_t const bitwidth);
    size_t get_bitwidth_orientation() const;

    void set_pareto_optimizer(bool const val);
    bool get_pareto_optimizer() const;

    void set_lex_optimizer(bool const val);
    bool get_lex_optimizer() const;

    void set_box_optimizer(bool const val);
    bool get_box_optimizer() const;

    void set_parquet_fp(bool const val);
    bool get_parquet_fp() const;

    void set_results_id(size_t const id);
    size_t get_results_id() const;

    void set_partitioning(bool const val);
    bool get_partitioning() const;

    void set_partition_size(size_t const val);
    size_t get_partition_size() const;

    size_t get_partition_id();

    void set_logic(eLogic const logic);
    eLogic get_logic() const;

    void store_constraint(z3::expr const & constraint);
    z3::expr_vector get_stored_constraints();

    void set_free_terminals(bool const value);
    bool get_free_terminals() const;

    void set_database_file(std::string const & name);
    std::string get_database_file();
    std::string get_database_dir();
    
    std::string orientation_to_string(eOrientation const orientation);

    static z3::context m_z3_ctx;

private:
    static std::vector<std::string> m_lef;
    static std::string m_def;
    static std::string m_site;
    static std::string m_supplement;
    static std::string m_working_directory;
    static std::string m_results_directory;
    static std::string m_image_directory;
    static std::string m_smt_directory;
    static std::string m_parquet_directory;
    static std::string m_log_name;
    static std::string m_bookshelf_file;
    static std::string m_design_name;
    static std::string m_binary_name;
    static std::string m_database_file;
    static size_t m_timeout;
    static size_t m_bitwidth_orientation;
    static size_t m_partition_id;
    static bool m_verbose;
    static bool m_log;
    static bool m_min_die_mode;
    static bool m_allsat;
    static bool m_smt_to_filesystem;
    static bool m_save_all;
    static bool m_save_best;
    static bool m_dump_all;
    static bool m_dump_best;
    static bool m_store_smt;
    static bool m_pareto_optimizer;
    static bool m_lex_optimizer;
    static bool m_box_optimizer;
    static bool m_parquet_fp;
    static bool m_partitioning;
    static bool m_free_terminals;
    static size_t m_partition_size;
    static size_t m_solutions;
    static size_t m_results_id;
    static eLogic m_logic;
    static z3::expr_vector m_stored_constraints;
};

}

#endif /* OBJECT_HPP */
