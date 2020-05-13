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

#include <object.hpp>
#include <utils.hpp>
#include <sstream>
#include <ctime>
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
   
   void dump_all();
   void solver_version(std::string const & version);
   
   void supplement_file(std::string const & file);
   
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
   void solve_allsat();
   
   void unsat_solution();
   
   void use_timeout(size_t const sec);
   
   void start_encoding();
   void end_encoding();
   
   void construct_tree(size_t const edges);
   
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
   
   void place_macro(std::string const & name,
                    size_t const lx,
                    size_t const ly,
                    size_t const orientation);
   
   void run_partitioning();
private:
    Logger();
    virtual ~Logger();

    static Logger* p_instance;
    static bool p_singleton;
};

}

#endif /* LOGGER_HPP */
