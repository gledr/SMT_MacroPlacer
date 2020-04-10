//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : object.cpp
//
// Date         : 07.January 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Virtual Base Class
//==================================================================
#include "object.hpp"

using namespace Placer;

std::string Object::m_def;
std::string Object::m_site;
std::string Object::m_supplement;
std::string Object::m_working_directory;
std::string Object::m_results_directory;
std::string Object::m_image_directory;
std::string Object::m_smt_directory;
std::string Object::m_log_name;
std::string Object::m_parquet_directory;
std::string Object::m_bookshelf_file;
std::string Object::m_design_name;
std::string Object::m_binary_name;
std::vector<std::string> Object::m_lef;
bool Object::m_verbose = false;
bool Object::m_log = false;
bool Object::m_min_die_mode = false;
bool Object::m_allsat;
bool Object::m_smt_to_filesystem = false;
bool Object::m_save_all = false;
bool Object::m_save_best = false;
bool Object::m_dump_all = false;
bool Object::m_dump_best = false;
bool Object::m_store_smt = false;
bool Object::m_pareto_optimizer = false;
bool Object::m_lex_optimizer = false;
bool Object::m_box_optimizer = false;
bool Object::m_partitioning = false;
bool Object::m_parquet_fp = false;
size_t Object::m_timeout = 0;
size_t Object::m_bitwidth_orientation;
size_t Object::m_solutions = 1;
size_t Object::m_results_id = 0;
size_t Object::m_partition_id = 0;
size_t Object::m_partition_size = 1;
eLogic Object::m_logic = eInt;
z3::context* Object::m_z3_ctx = new z3::context();
z3::expr_vector Object::m_stored_constraints(*m_z3_ctx);

Object::Object()
{
}

Object::~Object()
{
}

void Object::add_def(std::string const & def)
{
    m_def = def;
}

void Object::add_lef(std::string const & lef)
{
    m_lef.push_back(lef);
}

std::string& Object::get_def() const
{
    return m_def;
}

std::vector<std::string>& Object::get_lef() const
{
    return m_lef;
}

std::string& Object::get_site() const
{
    return m_site;
}

void Object::set_site(std::string const & site)
{
    m_site = site;
}

std::string& Object::get_supplement() const
{
    return m_supplement;
}

void Object::set_supplement(std::string const & file)
{
    m_supplement = file;
}

void Object::set_working_directory(std::string const & dir)
{
    m_working_directory = dir;
}

std::string& Object::get_working_directory() const
{
    return m_working_directory;
}

std::string& Object::get_results_directory() const
{
    return m_results_directory;
}

void Object::set_results_directory(std::string const & name)
{
    m_results_directory = m_working_directory + "/" + name;
}

std::string& Object::get_image_directory() const
{
    return m_image_directory;
}

void Object::set_image_directory(std::string const & name)
{
    m_image_directory = m_results_directory + "/" + std::to_string(m_results_id) + "/" + name;
}

bool Object::get_verbose() const
{
    return m_verbose;
}

void Object::set_verbose(bool const val)
{
    m_verbose = val;
}

bool Object::get_log_active() const
{
    return m_log;
}

void Object::set_log_active(bool const val)
{
    m_log = val;
}

std::string Object::get_log_directory()
{
    return m_results_directory + "/" + std::to_string(m_results_id);
}

std::string Object::get_smt_directory() const
{
    return m_smt_directory;
}

void Object::set_smt_directory(std::string const & name)
{
    m_smt_directory = m_results_directory + "/" + std::to_string(m_results_id) + "/" + name;
}

std::string Object::get_log_name() const
{
    return m_log_name;
}

void Object::set_log_name(std::string const & log_name)
{
    m_log_name = log_name;
}

std::string Object::get_parquet_directory() const
{
    return m_parquet_directory;
}

void Object::set_parquet_directory(std::string const & directory)
{
    m_parquet_directory = m_results_directory + "/" + std::to_string(m_results_id) + "/" + directory;
}

void Object::set_bookshelf_file(std::string const & file)
{
    m_bookshelf_file = file;
}

std::string Object::get_bookshelf_file() const
{
    return m_bookshelf_file;
}

size_t Object::get_timeout() const
{
    return m_timeout;
}

void Object::set_timeout(size_t const val)
{
    m_timeout = val;
}

bool Object::get_minimize_die_mode() const
{
    return m_min_die_mode;
}

void Object::set_minimize_die_mode(bool const val)
{
    m_min_die_mode = val;
}

bool Object::get_allsat() const
{
    return m_allsat;
}

size_t Object::get_max_solutions() const
{
    return m_solutions;
}

void Object::set_allsat(bool const val)
{
    m_allsat = val;
}

void Object::set_max_solutions(size_t const val)
{
    m_solutions = val;
}

bool Object::get_smt_to_filesystem() const
{
    return m_smt_to_filesystem;
}

void Object::set_smt_to_filesystem(bool const val)
{
    m_smt_to_filesystem = val;
}

bool Object::get_dump_all() const
{
    return m_dump_all;
}

bool Object::get_dump_best() const
{
    return m_dump_best;
}

bool Object::get_save_all() const
{
    return m_save_all;
}

bool Object::get_save_best() const
{
    return m_save_best;
}

void Object::set_dump_all(bool const val)
{
    m_dump_all = val;
}

void Object::set_dump_best(bool const val)
{
    m_dump_best = val;
}

void Object::set_save_best(bool const val)
{
    m_save_best = val;
}

void Object::set_save_all(bool const val)
{
    m_save_all = val;
}

bool Object::get_store_smt() const
{
    return m_store_smt;
}

void Object::set_store_smt(bool const val)
{
    m_store_smt = val;
}

std::string Object::get_design_name() const
{
    assert (!m_design_name.empty());
    return m_design_name;
}

void Object::set_design_name(std::string const & name)
{
    m_design_name = name;
}

void Object::set_binary_name(std::string const & name)
{
    m_binary_name = name;
}

std::string Object::get_binary_name() const
{
    return m_binary_name;
}

size_t Object::get_bitwidth_orientation() const
{
    return m_bitwidth_orientation;
}

void Object::set_bitwidth_orientation(size_t const bitwidth)
{
    m_bitwidth_orientation = bitwidth;
}

bool Object::get_pareto_optimizer() const
{
    return m_pareto_optimizer;
}

void Object::set_pareto_optimizer(bool const val)
{
    m_pareto_optimizer = val;
}

bool Object::get_lex_optimizer() const
{
    return m_lex_optimizer;
}

void Object::set_lex_optimizer(bool const val)
{
    m_lex_optimizer = val;
}

bool Object::get_box_optimizer() const
{
    return m_box_optimizer;
}

void Object::set_box_optimizer(bool const val)
{
    m_box_optimizer = val;
}

bool Object::get_parquet_fp() const
{
    return m_parquet_fp;
}

void Object::set_parquet_fp(bool const val)
{
    m_parquet_fp = val;
}

size_t Object::get_results_id() const
{
    return m_results_id;
}

void Object::set_results_id(size_t const id)
{
    m_results_id = id;
}

bool Object::get_partitioning() const
{
    return m_partitioning;
}

void Object::set_partitioning(bool const val)
{
    m_partitioning = val;
}

size_t Object::get_partition_id()
{
    size_t id = m_partition_id;
    m_partition_id++;

    return id;
}

size_t Object::get_partition_size() const
{
    return m_partition_size;
}

void Object::set_partition_size(size_t const val)
{
    m_partition_size = val;
}

eLogic Object::get_logic() const
{
    return m_logic;
}

void Object::set_logic(eLogic const logic)
{
    m_logic = logic;
}

void Object::store_constraint(z3::expr const & constraint)
{
    m_stored_constraints.push_back(constraint);
}

z3::expr_vector Object::get_stored_constraints()
{
    return m_stored_constraints;
}
