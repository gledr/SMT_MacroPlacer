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
std::string Object::m_mzn_directory;
std::string Object::m_log_name;
std::string Object::m_parquet_directory;
std::string Object::m_bookshelf_file;
std::string Object::m_bookshelf_export;
std::string Object::m_design_name;
std::string Object::m_binary_name;
std::string Object::m_database_file;
std::string Object::m_db_to_csv_script;
std::string Object::m_base_path;
std::string Object::m_ini_file;
std::vector<std::string> Object::m_lef;
bool Object::m_verbose = false;
bool Object::m_log = false;
bool Object::m_min_die_mode = false;
bool Object::m_min_hpwl_mode = false;
bool Object::m_smt_to_filesystem = false;
bool Object::m_save_all = false;
bool Object::m_save_best = false;
bool Object::m_dump_all = false;
bool Object::m_dump_best = false;
bool Object::m_store_smt = false;
bool Object::m_z3_api_mode = false;
bool Object::m_pareto_optimizer = false;
bool Object::m_lex_optimizer = false;
bool Object::m_minizinc_mode = false;
bool Object::m_partitioning = false;
bool Object::m_parquet_fp = false;
bool Object::m_free_terminals = false;
bool Object::m_strip_terminals = false;
bool Object::m_skip_power_network = false;
size_t Object::m_timeout = 0;
size_t Object::m_bitwidth_orientation;
size_t Object::m_solutions = 1;
size_t Object::m_results_id = 0;
size_t Object::m_partition_id = 0;
size_t Object::m_partition_size = 1;
size_t Object::m_num_partitions = 1;
size_t Object::m_key_counter = 0;
size_t Object::m_alpha_weight = 0;
size_t Object::m_beta_weight = 0;
eLogic Object::m_logic = eInt;

z3::context Object::m_z3_ctx(init_context());
z3::expr_vector Object::m_stored_constraints(m_z3_ctx);


Object::Object()
{

}

Object::~Object()
{
}

z3::context Object::init_context()
{
    z3::config cfg;
   
    return z3::context(cfg);
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

std::string Object::get_active_results_directory()
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

std::string Object::get_mzn_directory() const
{
    return m_mzn_directory;
}

void Object::set_mzn_directory(std::string const & name)
{
    m_mzn_directory = m_results_directory + "/" + std::to_string(m_results_id) + "/" + name;
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

std::string Object::get_bookshelf_export() const
{
    return m_bookshelf_export;
}

void Object::set_bookshelf_export(std::string const & name)
{
    m_bookshelf_export = name;
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

bool Object::get_minimize_hpwl_mode() const
{
    return m_min_hpwl_mode;
}

void Object::set_minimize_hpwl_mode(bool const val)
{
    m_min_hpwl_mode = val;
}

size_t Object::get_max_solutions() const
{
    return m_solutions;
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

bool Object::get_z3_api_mode() const
{
    return m_z3_api_mode;
}

void Object::set_z3_api_mode(bool const val)
{
    m_z3_api_mode = val;
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

bool Object::get_parquet_fp() const
{
    return m_parquet_fp;
}

void Object::set_parquet_fp(bool const val)
{
    m_parquet_fp = val;
}

bool Object::get_minizinc_mode() const
{
    return m_minizinc_mode;
}

void Object::set_minizinc_mode(bool const val)
{
    m_minizinc_mode = val;
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

size_t Object::get_num_partitions() const 
{
    return m_num_partitions;
}

void Object::set_num_partition(size_t const num)
{
    m_num_partitions = num;
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

bool Object::get_free_terminals() const
{
    return m_free_terminals;
}

void Object::set_free_terminals(bool const value)
{
    m_free_terminals = value;
}

void Object::set_strip_terminals(bool const val)
{
    m_strip_terminals = val;
}

bool Object::get_strip_terminals() const
{
    return m_strip_terminals;
}

std::string Object::get_database_file()
{
    return m_database_file;
}

void Object::set_database_file(std::string const & name)
{
    m_database_file = name;
}

std::string Object::get_database_dir()
{
    return m_results_directory + "/" + std::to_string(m_results_id) + "/";
}

std::string Object::get_db_to_csv_script() const
{
    return m_db_to_csv_script;
}

void Object::set_db_to_csv_script(std::string const & url)
{
    m_db_to_csv_script = url;
}

std::string Object::get_base_path() const
{
    return m_base_path;
}

void Object::set_base_path(std::string const & path)
{
    m_base_path = path;
}

std::string Object::orientation_to_string(eOrientation const orientation)
{
    if (orientation == eNorth){
        return "N";
    } else if (orientation == eWest){
        return "W";
    } else if (orientation == eSouth){
        return "S";
    } else if (orientation == eEast){
        return "E";
    } else if (orientation == eFlipNorth){
        return "FN";
    } else if (orientation == eFlipWest){
        return "FW";
    } else if (orientation == eFlipSouth){
        return "FS";
    } else if (orientation == eFlipEast){
        return "FE";
    } else {
        notsupported_check("Orientation not Supported!");
    }

    return ""; // Make Compiler Happy
}

bool Object::get_skip_power_network() const
{
    return m_skip_power_network;
}

void Object::set_skip_power_network(bool const val)
{
    m_skip_power_network = val;
}

std::string Object::get_ini_file() const
{
    return m_ini_file;
}

void Object::set_ini_file(std::string const & file)
{
    m_ini_file = file;
}

std::string Object::get_third_party_bin()
{
    return this->get_base_path() + "/05_third_party/bin/";
}

std::string Object::get_z3_bin()
{
#ifdef BUILD_Z3
    return this->get_third_party_bin() + "/z3";
# else 
    return "z3";
#endif
}

std::string Object::get_minizinc_bin()
{
    return this->get_third_party_bin() + "/minizinc";
}

std::string Object::get_or_tools_bin()
{
    return this->get_third_party_bin() + "/fzn-or-tools";
}

size_t Object::get_alpha_weight() const
{
    return m_alpha_weight;
}

size_t Object::get_beta_weight() const
{
    return m_beta_weight;
}

void Object::set_alpha_weight(size_t const val)
{
    m_alpha_weight = val;
}

void Object::set_beta_weight(size_t const val)
{
    m_beta_weight = val;
}
