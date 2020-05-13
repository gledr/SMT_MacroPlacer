//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : logger.cpp
//
// Date         : 07.January 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Logger Class
//==================================================================
#include "logger.hpp"

using namespace Placer;
using namespace Placer::Utils;


Logger* Logger::p_instance = nullptr;
bool Logger::p_singleton = false;

Logger* Logger::getInstance()
{
    if(Logger::p_singleton == false){
        Logger::p_instance = new Logger();
        Logger::p_singleton = true;
    }
    
    return p_instance;
}

void Logger::destroy()
{
    Logger::p_singleton= false;
    delete Logger::p_instance; Logger::p_instance = nullptr;
}

Logger::Logger():
    BaseLogger(), Object()
{
    p_log_stream->set_enabled(true);
    p_log_stream->set_quiet(false);

    p_log_stream->set_dump_to_shell(this->get_verbose());
    p_log_stream->set_dump_to_file(this->get_log_active());

    p_log_stream->set_working_directory(this->get_log_directory());
    p_log_stream->set_file_name(this->get_log_name());
}

Logger::~Logger()
{
}

void Logger::supplement_file(std::string const & file)
{
    LOG(eInfo) << "Using Supplement File: " + file;
}

void Logger::start_macro_thread()
{
    LOG(eDebug) << "Thread 'macro_worker' fired";
}

void Logger::end_macro_thread()
{
    LOG(eDebug) << "Thread 'macro_worker' terminated";
}

void Logger::start_cell_thread()
{
    LOG(eDebug) << "Thread 'cell_worker' fired";
}

void Logger::end_cell_thread()
{
    LOG(eDebug) << "Thread 'cell_worker' terminated";
}

void Logger::start_terminal_thread()
{
    LOG(eDebug) << "Thread 'terminal_worker' fired";
}

void Logger::end_terminal_thread()
{
    LOG(eDebug) << "Thread 'terminal_worker' terminated";
}

void Logger::timer_total(size_t const ms)
{
    LOG(eInfo) << "SMT Solver Total Time Spent: " + std::to_string(ms) + " ms";
}

void Logger::timer_single_solution(std::string const & id, size_t const ms)
{
}

void Logger::solve_allsat()
{
}

void Logger::solve_solution()
{
    LOG(eDebug) << "Solve Single Solution";
}

void Logger::use_timeout(size_t const sec)
{
    std::stringstream msg;
    msg <<  "Setting Timeout for Solver " + std::to_string(sec) << " sec";
    LOG(eInfo) << msg.str();
}

void Logger::end_encoding()
{
    LOG(eInfo) << "Encoding Done";
}

void Logger::start_encoding()
{
    LOG(eInfo) << "Start Encoding";
}

void Logger::construct_tree(size_t const edges)
{
    std::stringstream msg;
    msg  << "Constructing Tree from " << edges << " edges...";
    LOG(eDebug) << msg.str();
}

void Logger::min_die_area(double const & area)
{ 
    std::stringstream msg;
    msg <<  "Minimum Die Area: " << area << " mm2";
    LOG(eInfo) << msg.str();
}

void Logger::result_die_area(double const & area)
{
    std::stringstream msg;
    msg <<  "Estimated Die Area: " << area << " mm2";
    LOG(eInfo) << msg.str();
}

void Logger::white_space(double const & val)
{
    std::stringstream msg;
    msg <<  "Whitespace Area: " << val << " %";
    LOG(eInfo) << msg.str();
}
void Logger::unsat_solution()
{
    LOG(eInfo) << "UNSAT: No valid Solution could be found!";
}

void Logger::solve_optimize()
{
    LOG(eInfo) << "Solving Optimization Problem...";
}   

void Logger::use_pareto_optimizer()
{
    LOG(eInfo) << "Using Pareto Optimization";
}

void Logger::use_lex_optimizer ()
{
    LOG(eInfo) << "Using Lex Optimization";
}

void Logger::use_box_optimizer ()
{
    LOG(eInfo) << "Using Box Optimization";
}

void Logger::pareto_step()
{
  LOG(eInfo) << "Next Pareto Optimization Step";
}

void Logger::set_die_ux(size_t const val)
{
    std::stringstream msg;
    msg << "Setting Die ux to " << val;
    LOG(eInfo) << msg.str();
}
  
void Logger::set_die_uy(size_t const val)
{
    std::stringstream msg;
    msg << "Setting Die uy to " << val;
    LOG(eInfo) << msg.str();
}

void Logger::set_die_lx(size_t const val)
{
    std::stringstream msg;
    msg << "Setting Die lx to " << val;
    LOG(eInfo) << msg.str();
}
  
void Logger::set_die_ly(size_t const val)
{
    std::stringstream msg;
    msg << "Setting Die ly to " << val;
    LOG(eInfo) << msg.str();
}

void Logger::place_macro(std::string const & name,
                         size_t const lx,
                         size_t const ly,
                         size_t const orientation)
{
    std::stringstream msg;
    msg << "Placing " << name << ": " << lx << "-" << ly;
    
     if(orientation == 0){
        msg << " for orientation: North";
    } else if (orientation == 1){
         msg << " for orientation: West";
    } else if (orientation == 2){
         msg << " for orientation: South";
    } else if (orientation == 3){
      msg << " for orientation: Est";
    } else {
        assert (0);
    }
    LOG(eDebug) << msg.str();
}

void Logger::add_solution_layout(size_t const ux, size_t const uy)
{
    std::stringstream msg;
    msg << "Setting Layout to: " << ux << "x" << uy;
    LOG(eInfo) << msg.str();
}

void Logger::pareto_solutions (size_t const solutions)
{
    std::stringstream msg;
    msg <<  "Max Pareto Solutions: " << solutions;
    LOG(eInfo) << msg.str();
}

void Logger::dump_all ()
{
    LOG(eInfo) << "Exporting All Placement Solutions as PNG";
}

void Logger::solver_version (std::string const & version)
{
    std::stringstream msg;
    msg << "Using Solver " << version;
    LOG(eInfo) << msg.str();
}

void Logger::execution_start ()
{
    std::stringstream msg;
    msg <<  "Execution Start: " << Utils::Utils::get_current_time();
    LOG(eDebug) << Utils::Utils::get_bash_string_blink_red(msg.str());
}

void Logger::execution_end ()
{
    std::stringstream msg;
    msg <<  "Execution End: " << Utils::Utils::get_current_time();
    LOG(eDebug) << Utils::Utils::get_bash_string_blink_red(msg.str());
}

void Logger::run_partitioning()
{
    LOG(eInfo) << "Partitioning Mode!";
}

void Logger::encode_int()
{
    LOG(eInfo) << "Using LIA Theory";
}

void Logger::encode_bv()
{
    LOG(eInfo) << "Using QF_BV Theory";
}
