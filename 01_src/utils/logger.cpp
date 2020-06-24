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
    BaseLogger(),
    Object()
{
    p_log_stream->set_enabled(true);
    p_log_stream->set_quiet(false);

    p_log_stream->set_dump_to_shell(this->get_verbose());
    p_log_stream->set_dump_to_file(this->get_log_active());

    p_log_stream->set_working_directory(this->get_active_results_directory());
    p_log_stream->set_file_name(this->get_log_name());
}

Logger::~Logger()
{
}

void Logger::add_free_macro(std::string const & id,
                            size_t const w,
                            size_t const h)
{
    std::stringstream msg;
    msg << "Create Free Macro " << id << " (" << w << ":" << h << ")";

    LOG(eInfo) << msg.str();
}

void Logger::add_fixed_macro(std::string const & id)
{
    std::stringstream msg;
    msg << "Create Fixed Macro: " << id;

    LOG(eInfo) << msg.str();
}

void Logger::add_supplement_macro(std::string const & id)
{
    LOG(eInfo) << "Adding Supplement Macro (" << id << ")";
}

void Logger::supplement_file(std::string const & file)
{
    LOG(eInfo) << "Using Supplement File: " + file;
}

void Logger::write_supplement(std::string const & file)
{
    LOG(eInfo) << "Writing Supplement File: " + file;
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

void Logger::insert_edge(std::string const & from, std::string const & to)
{
    std::stringstream msg;
    msg << "Insert Edge: " << from << " -> " << to;
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

void Logger::unknown_solution()
{
    LOG(eInfo) << "UNKNOWN: No Solution Found! Check Timeout and Memory Limits!";
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
                         eOrientation const orientation)
{
    std::stringstream msg;
    msg << "Placing " << name << ": " << lx << "-" << ly 
        << " for orientation " << this->orientation_to_string(orientation);

    LOG(eDebug) << msg.str();
}

void Logger::place_terminal(std::string const & name,
                            size_t const x,
                            size_t const y)
{
    std::stringstream msg;
    msg << "Placing Terminal " << name << ": " << x << "-" << y ;

    LOG(eDebug) << msg.str();
}

void Logger::add_solution_layout(size_t const ux, size_t const uy)
{
    std::stringstream msg;
    msg << "Setting Layout to: " << ux << "x" << uy;
    LOG(eInfo) << msg.str();
}

void Logger::add_solution_die_ux(size_t const val)
{
    std::stringstream msg;
    msg << "Add Solution Layout UX: " << val;

    LOG(eInfo) << msg.str();
}

void Logger::add_solution_die_uy(size_t const val)
{
        std::stringstream msg;
    msg << "Add Solution Layout UY: " <<  val;

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

void Logger::dump_best()
{
    LOG(eInfo) << "Exporting Best Placement Solution as PNG";
}

void Logger::save_all (eInputFormat const type)
{
    if (type == eLEFDEF){
        LOG(eInfo) << "Exporting all Placement Solutions as LEF/DEF";
    } else if (type == eBookshelf){
        LOG(eInfo) << "Exporting all Placement Solutions as Bookshelf";
    } else {
        assert (0);
    }
}

void Logger::save_best(eInputFormat const type)
{
    if (type == eLEFDEF){
        LOG(eInfo) << "Exporting Best Placement Solutions as LEF/DEF";
    } else if (type == eBookshelf){
        LOG(eInfo) << "Exporting Best Placement Solutions as Bookshelf";
    } else {
        assert (0);
    }
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

void Logger::add_fixed_terminal(std::string const & name,
                                size_t const x,
                                size_t const y)
{
    std::stringstream msg;
    msg << "Add Fixed Terminal (" << name << ") at " << x << ":" << y;
    LOG(eDebug) << msg.str();
}

void Logger::add_free_terminal(std::string const & name)
{
    std::stringstream msg;
    msg << "Add Free Terminal (" << name << ")";
    LOG(eDebug) << msg.str();
}

void Logger::print_header(std::stringstream const & header)
{
    LOG(eStdOut) << header.str();
}

void Logger::print_version(std::string const & date,
                           std::string const & commit,
                           std::string const& author,
                           size_t const max_len)
{
    size_t const outline = 14;
    std::stringstream date_stream;
    std::stringstream commit_stream;
    std::stringstream author_stream;

    commit_stream << std::left << std::setw(outline) << std::setfill(' ') << "Git Hash: "  << commit;
    author_stream << std::left << std::setw(outline) << std::setfill(' ') << "Last Author: "  << author;
    date_stream << std::left << std::setw(outline) << std::setfill(' ') << "Last Changes: " << date;
    
    LOG(eStdOut) << commit_stream.str();
    LOG(eStdOut) << author_stream.str();
    LOG(eStdOut) << date_stream.str();
    LOG(eStdOut) << std::string (max_len, '-');
    LOG(eStdOut) << "";
}

void Logger::export_db_to_csv(std::string const & filename)
{
    std::stringstream msg;
    msg << "Exporting Database to CSV (" << filename << ")";

    LOG(eInfo) << msg.str();
}

void Logger::export_hypergraph(std::string const & filename)
{
    std::stringstream msg;
    msg << "Exporting Hyphergraph (" << filename << ")";

    LOG(eInfo) << msg.str();
}

void Logger::deduce_layout(size_t const x, size_t const y)
{
    std::stringstream msg;
    msg << "Layout Deduced From Terminals (" << x << ":" << y << ")";

    LOG(eInfo) << msg.str();
}

void Logger::bookshelf_read_aux(std::string const & filename)
{
    std::stringstream msg;
    msg << "Parsing Bookshelf Aux (" << filename << ")";

    LOG(eInfo) << msg.str();
}

void Logger::bookshelf_read_nets(std::string const & filename)
{
    std::stringstream msg;
    msg << "Parsing Bookshelf Nets (" << filename << ")";

    LOG(eInfo) << msg.str();
}

void Logger::bookshelf_read_blocks(std::string const & filename)
{
    std::stringstream msg;
    msg << "Parsing Bookshelf Blocks (" << filename << ")";

    LOG(eInfo) << msg.str();
}

void Logger::bookshelf_read_place(std::string const & filename)
{
    std::stringstream msg;
    msg << "Parsing Bookshelf Place (" << filename << ")";

    LOG(eInfo) << msg.str();
}

void Logger::bookshelf_write_aux(std::string const & filename)
{
    std::stringstream msg;
    msg << "Exporting Bookshelf Aux (" << filename << ")";

    LOG(eInfo) << msg.str();
}

void Logger::bookshelf_write_nets(std::string const & filename)
{
    std::stringstream msg;
    msg << "Exporting Bookshelf Nets (" << filename << ")";

    LOG(eInfo) << msg.str();
}

void Logger::bookshelf_write_blocks(std::string const & filename)
{
    std::stringstream msg;
    msg << "Exporting Bookshelf Blocks (" << filename << ")";

    LOG(eInfo) << msg.str();
}

void Logger::bookshelf_write_place(std::string const & filename)
{
    std::stringstream msg;
    msg << "Exporting Bookshelf Place (" << filename << ")";

    LOG(eInfo) << msg.str();
}

void Logger::start_kahypar()
{
    LOG(eInfo) << "Kahypar Hyphergraph Partitioning started...";
}

void Logger::kahypar_finished()
{
    LOG(eInfo) << "Kahypar Hyphergraph Partitioning finished...";
}

void Logger::strip_terminals()
{
    LOG(eInfo) << "Stripping Terminals from Circuit...";
}

void Logger::place_pin_x(std::string const & pin_name,
                     std::string const & parent_name,
                     size_t const pos)
{
    std::stringstream msg;
    msg << "Place Pin X " << pin_name << "(" << parent_name << ") to " << pos;
    
    LOG(eDebug) << msg.str();
}

void Logger::place_pin_y(std::string const & pin_name,
                 std::string const & parent_name,
                 size_t const pos)
{
    std::stringstream msg;
    msg << "Place Pin Y " << pin_name << "(" << parent_name << ") to " << pos;
    
    LOG(eDebug) << msg.str();
}

void Logger::place_terminal_x(std::string const & terminal_name,
                              size_t const pos)
{
    std::stringstream msg;
    msg << "Place Terminal X " << terminal_name << " to " << pos;

    LOG(eDebug) << msg.str();
}

void Logger::place_terminal_y(std::string const & terminal_name,
                              size_t const pos)
{
    std::stringstream msg;
    msg << "Place Terminal Y " << terminal_name << " to " << pos;
    
    LOG(eDebug) << msg.str();
}

void Logger::add_fixed_pin(std::string const & pin_name,
                           std::string const & parent_name,
                           size_t const x,
                           size_t const y)
{
    std::stringstream msg;
    msg << "Adding Fixed Pin " << pin_name << " to " << parent_name << " at (" << x << ":" << y << ")";
    
    LOG(eDebug) << msg.str();
}

void Logger::add_free_pin(std::string const & pin_name,
                          std::string const & parent_name)
{
    std::stringstream msg;
    msg << "Adding Free Pin " << pin_name << " to " << parent_name;

    LOG(eDebug) << msg.str();
}

void Logger::write_lef(std::string const & filename)
{
    LOG(eInfo) << std::string("Writing LEF file " + filename);
}

void Logger::write_def(std::string const & filename)
{
    LOG(eInfo) << std::string("Writing DEF file " + filename);
}
