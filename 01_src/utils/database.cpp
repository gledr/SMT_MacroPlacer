//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : database.cpp
//
// Date         : 15.May 2020
// Compiler     : gcc version 9.3.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Database Class to Store Results
//==================================================================
#include "database.hpp"

using namespace Placer::Utils;

/**
 * @brief Constructor
 * 
 * @param db_file Sqlite3 Database File
 */
Database::Database(std::string const & db_file):
    Object(),
    BaseDatabase(db_file)
{
    m_logger = Logger::getInstance();
}

/**
 * @brief Destructor
 */
Database::~Database()
{
    m_logger = nullptr;
}

/**
 * @brief Init Database and Create Tables
 */
void Database::init_database()
{
    std::stringstream stream;

    stream << "CREATE TABLE macros(";
    stream << "solution INTEGER,";
    stream << "type VARCHAR(50),";
    stream << "id VARCHAR(50),";
    stream << "free BOOLEAN,",
    stream << "lx INTEGER,",
    stream << "ly INTEGER,",
    stream << "orientation VARCHAR(50),",
    stream << "width INTEGER,",
    stream << "height INTEGER);";
    this->db_command(stream.str());

    stream.str("");
    stream << "CREATE TABLE terminals(";
    stream << "solution INTEGER,";
    stream << "id VARCHAR(50),";
    stream << "lx INTEGER,",
    stream << "ly INTEGER);",
    this->db_command(stream.str());

    stream.str("");
    stream << "CREATE TABLE layout(",
    stream << "solution INTEGER,";
    stream << "lx INTEGER,";
    stream << "ly INTEGER,";
    stream << "ux INTEGER,";
    stream << "uy INTEGER,";
    stream << "orientation VARCHAR(10));";
    this->db_command(stream.str());

    stream.str("");
    stream << "CREATE TABLE results(";
    stream << "solution INTEGER,";
    stream << "area INTEGER,";
    stream << "hpwl INTEGER);";
    this->db_command(stream.str());
}

/**
 * @brief Callback function used by the Sqlite3 API - has access to the this pointer
 * 
 * The azColName parameter is unused - since the interface is predefined we told GCC 
 * to ignore this.
 */ 
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
int Database::__callback__(int argc, char **argv, char **azColName)
{
#pragma GCC diagnostic pop
    if (*argv == nullptr){
        // If the Table is empty we receive a nullptr
        return 0;
    } else {
        if (p_active_transaction == e_init){
            // Chill dude
        }
        return 0;
    }
}

/**
 * @brief Insert Placed Macro
 * 
 * @param solution Solution ID
 * @param macro Pointer to Macro
 */
void Database::place_macro(size_t const solution, Macro* macro)
{
    assert (macro != nullptr);
    assert (macro->has_solution(solution));

    std::string s    = std::to_string(solution);
    std::string name = macro->get_name();
    std::string id   = macro->get_id();
    std::string free = std::to_string(macro->is_free());
    std::string lx   = std::to_string(macro->get_solution_lx(solution));
    std::string ly   = std::to_string(macro->get_solution_ly(solution));
    std::string w    = std::to_string(macro->get_width_numeral());
    std::string h    = std::to_string(macro->get_height_numeral());
    std::string o    = this->orientation_to_string(macro->get_solution_orientation(solution));

    std::stringstream query;
    query << "INSERT INTO macros VALUES (" << s << ",'" << name << "','"
          << id << "'," << free << ","  << lx << "," << ly << ",'" << o << "',"
          << w << "," << h << ");";

    this->db_command(query.str());
}

/**
 * @brief Insert Placed Terminal
 * 
 * @param solution Solution ID
 * @param terminal Pointer to Terminal
 */
void Database::place_terminal(size_t const solution, Terminal* terminal)
{
    assert (terminal != nullptr);

    if (terminal->has_solution(solution)){

        std::string sol  = std::to_string(solution);
        std::string name = terminal->get_name();
        std::string x    = std::to_string(terminal->get_solution_pos_x(solution));
        std::string y    = std::to_string(terminal->get_solution_pos_y(solution));

        std::stringstream query;
        query << "INSERT INTO terminals VALUES (" 
            << sol << ",'" << name << "'," << x << "," << y << ");";
        this->db_command(query.str());
    }
}

/**
 * @brief Export Database Content as CSV File
 * 
 * @param filename Filename to Store to
 */
void Database::export_as_csv(std::string const & filename)
{
    m_logger->export_db_to_csv(filename);

    std::string db = p_db_url;
    std::string csv = this->get_database_dir() + "/" + filename;
    std::string script = this->get_db_to_csv_script();

    std::vector<std::string> args;
    args.push_back(script);
    args.push_back(db);
    args.push_back(csv);

    Utils::Utils::system_execute("bash", args, "", false);
}
