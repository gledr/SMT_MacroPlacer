//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : bookshelf.hpp
//
// Date         : 13. January 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Bookshelf Input Reader
//==================================================================
#ifndef BOOKSHELF_HPP
#define BOOKSHELF_HPP

#include <object.hpp>
#include <utils.hpp>
#include <macro.hpp>
#include <terminal.hpp>
#include <tree.hpp>
#include <costfunction.hpp>

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

    void write_placement();

    size_t get_estimated_area();

    std::string get_design_name() const;

    std::vector<Macro*> get_macros();
    void set_macros(std::vector<Macro*> & macros);

    std::vector<Terminal*> get_terminals();
    void set_terminals(std::vector<Terminal*> & terminals);

    Tree* get_tree();
    void set_tree(Tree * tree);

private:
    CostFunction* m_lut;
    
    void read_blocks();
    void read_nets();
    void read_pl();

    void write_blocks();
    void write_nets();
    void write_pl();
    
    size_t m_gcd_h;
    size_t m_gcd_w;
    
    size_t m_max_h;
    size_t m_max_w;

    std::string m_design_name;
    std::string m_blocks_file;
    std::string m_nets_file;
    std::string m_pl_file;
    size_t m_estimated_area;

    std::vector<Macro*> m_macros;
    std::vector<Terminal*> m_terminals;
    Tree* m_tree;

    /* Name, Width, Height */
    std::map<std::string, std::pair<size_t, size_t>> m_macro_definitions;

    /* Name, x, y */
    std::map<std::string, std::pair<size_t, size_t>> m_terminal_definitions;

    size_t m_expected_macros;
    size_t m_excepted_terminals;

    void calc_estimated_die_area();
    void add_pin_to_macro(std::string const & macro,
                          std::string const & pin,
                          std::string const & direction,
                          std::string const & pos_x,
                          std::string const & pos_y);

    Macro* find_macro(std::string const & name);
    Terminal* find_terminal(std::string const & name);
    
    void calculate_gcd();
    void locate_biggest_macro();
};

}

#endif /* BOOKSHELF_HPP */
