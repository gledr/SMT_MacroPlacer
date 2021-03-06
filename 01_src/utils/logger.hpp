//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : logger.hpp
//
// Date         : 07.January 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Logger Class
//==================================================================
#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iomanip>
#include <sstream>
#include <ctime>

#include <object.hpp>
#include <utils.hpp>
#include <base_logger.hpp>

#include <boost/filesystem.hpp>

namespace Placer::Utils {

/**
 * @class Logger
 * 
 * @brief Logging Class based on BaseLogger
 */
class Logger: public virtual Placer::Object,
              public virtual Placer::Utils::BaseLogger {
public:
    static Logger* getInstance();

    static void destroy();

    void execution_start();
    void execution_end();

    void add_free_macro(std::string const & id,
                        size_t const w,
                        size_t const h);
    void add_fixed_macro(std::string const & id);

    void add_supplement_macro(std::string const & id);

    void dump_all();
    void dump_best();

    void save_all(eInputFormat const type);
    void save_best(eInputFormat const type);

    void solver_version(std::string const & version);
    void def_units(size_t const value);
    void lef_units(size_t const value);

    void supplement_file(std::string const & file);
    void write_supplement(std::string const & file);

    void start_macro_thread();
    void end_macro_thread();

    void start_cell_thread();
    void end_cell_thread();

    void start_terminal_thread();
    void end_terminal_thread();

    void timer_total(size_t const ms);
    void timer_single_solution(std::string const & id, size_t const ms);

    void solve_optimize();
    void solve_solution();
    void invoke_heuristics_lab();

    void z3_api_mode();
    void z3_shell_mode();
    void optimathsat_shell_mode();

    void unsat_solution();
    void unknown_solution();
    void solver_timeout();

    void use_timeout(size_t const sec);

    void start_encoding();
    void end_encoding();

    void construct_tree(size_t const edges);
    void insert_edge(std::string const & from, std::string const & to);

    void min_die_area(double const & area);
    void result_die_area(double const & area);
    void white_space(double const & space);

    void use_pareto_optimizer();
    void use_lex_optimizer();
    void use_box_optimizer();

    void encode_int();
    void encode_bv();

    void pareto_solutions(size_t const solutions);
    void pareto_step();

    void set_die_ux(size_t const val);
    void set_die_uy(size_t const val);
    void set_die_lx(size_t const val);
    void set_die_ly(size_t const val);
    void add_solution_layout(size_t const ux, size_t const uy);
    void add_solution_die_ux(size_t const val);
    void add_solution_die_uy(size_t const val);

    void place_macro(std::string const & name,
                    size_t const lx,
                    size_t const ly,
                    eOrientation const orientation);

    void place_pin_x(std::string const & pin_name,
                     std::string const & parent_name,
                     size_t const pos);

    void place_pin_y(std::string const & pin_name,
                     std::string const & parent_name,
                     size_t const pos);

    void place_terminal_x(std::string const & terminal_name,
                          size_t const pos);

    void place_terminal_y(std::string const & terminal_name,
                          size_t const pos);

    void place_terminal(std::string const & name,
                       size_t const x,
                       size_t const y);

    void run_partitioning();
    void deduce_layout(size_t const x, size_t const y);

    void add_free_terminal(std::string const & name);
    void add_fixed_terminal(std::string const & name,
                           size_t const x,
                           size_t const y);

    void add_free_pin(std::string const & pin_name,
                      std::string const & parent_name);

    void add_fixed_pin(std::string const & pin_name,
                       std::string const & parent_name,
                       size_t const x,
                       size_t const y);

    void export_db_to_csv(std::string const & filename);
    void export_hypergraph(std::string const & filename);
    void print_header(std::stringstream const & header);
    void print_footer(std::stringstream const & footer);
    void print_version(std::string const & date,
                      std::string const & commit,
                      std::string const & author,
                      size_t const max_len);

    void bookshelf_read_aux(std::string const & filename);
    void bookshelf_read_nets(std::string const & filename);
    void bookshelf_read_blocks(std::string const & filename);
    void bookshelf_read_place(std::string const & filename);

    void bookshelf_write_aux(std::string const & filename);
    void bookshelf_write_nets(std::string const & filename);
    void bookshelf_write_blocks(std::string const & filename);
    void bookshelf_write_place(std::string const & filename);

    void start_kahypar();
    void kahypar_finished();

    void strip_terminals();

    void write_lef(std::string const & filename);
    void write_def(std::string const & filename);
    
    void connection_retry();
    void connect_to_backend(std::string const & ip, int const port);
    void connected();
    
    void send(size_t const bytes);
    void receive(size_t const bytes);
    
    void start_serialize();
    void end_serialize();
    
    void start_deserialize();
    void end_deserialize();

private:
    Logger();
    virtual ~Logger();

    static Logger* p_instance;
    static bool p_singleton;
};

} /* namespace Placer::Utils */

#endif /* LOGGER_HPP */
