//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : bookshelf.hpp
//
// Date         : 13. January 2019
// Compiler     : gcc version 10.0.1 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Bookshelf Input Reader
//==================================================================
#ifndef BOOKSHELF_HPP
#define BOOKSHELF_HPP

#include <object.hpp>
#include <macro_definition.hpp>
#include <terminal_definition.hpp>
#include <utils.hpp>
#include <macro.hpp>
#include <terminal.hpp>
#include <tree.hpp>
#include <logger.hpp>
#include <exception.hpp>

#include <boost/filesystem.hpp>
#include <z3++.h>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <exception>
#include <algorithm>

namespace Placer {

/**
 * @class Bookshelf
*  @brief Read/Write Bookshelf Layout Data
 */
class Bookshelf: public virtual Object{
public:
    Bookshelf();

    virtual ~Bookshelf();

    void read_files();

    void write_placement(size_t const solution_id = 0);

    void strip_terminals();

    size_t get_estimated_area();

    std::pair<size_t, size_t> get_deduced_layout();
    bool could_deduce_layout();

    std::string get_design_name() const;

    std::vector<Macro*> get_macros();
    void set_macros(std::vector<Macro*> & macros);

    std::vector<Terminal*> get_terminals();
    void set_terminals(std::vector<Terminal*> & terminals);

    Tree* get_tree();
    void set_tree(Tree * tree);

private:
    void read_aux();
    void read_blocks();
    void read_nets();
    void read_pl();

    void write_aux();
    void write_blocks();
    void write_nets();
    void write_pl(size_t const solution_id);

    std::string m_design_name;
    std::string m_blocks_file;
    std::string m_nets_file;
    std::string m_pl_file;
    size_t m_estimated_area;
    bool m_design_read;

    std::vector<Macro*> m_macros;
    std::vector<MacroDefinition> m_macro_definitions;
    std::vector<Terminal*> m_terminals;
    std::vector<TerminalDefinition> m_terminal_definitions;
    Tree* m_tree;
    Utils::Logger* m_logger;

    size_t m_expected_macros;
    size_t m_excepted_terminals;
    std::pair<size_t, size_t> m_deduced_layout;
    bool m_could_duduce_layout;

    void calc_estimated_die_area();
    void add_pin_to_macro(std::string const & macro,
                          std::string const & pin,
                          std::string const & direction,
                          std::string const & pos_x,
                          std::string const & pos_y);

    Macro* find_macro(std::string const & name);
    bool has_macro(std::string const & name);

    Terminal* find_terminal(std::string const & name);
    bool has_terminal(std::string const & name);

    void deduce_layout();
    bool check_name_dac2002(std::string const & name);
};

} /* namespace Placer */

#endif /* BOOKSHELF_HPP */
