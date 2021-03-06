//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : supplement.hpp
//
// Date         : 09. January 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : LEF/DEF Supplement File
//==================================================================
#ifndef SUPPLEMENT_HPP
#define SUPPLEMENT_HPP

#include <object.hpp>
#include <logger.hpp>
#include <exception.hpp>
#include <supplementmacro.hpp>
#include <supplementlayout.hpp>
#include <supplementterminal.hpp>

#include <json/json.h>
#include <unordered_map>
#include <iostream>

#include <boost/filesystem.hpp>

namespace Placer {

/**
 * @class Supplement
 * @brief Add Additional Information not Supported by LEF/DEF Standard
 */
class Supplement: public virtual Object{
public:

    Supplement();

    ~Supplement();

    void read_supplement_file();

    void write_supplement_file();

    bool has_supplement();

    bool has_macro(std::string const & id);

    bool has_terminal(std::string const & id);

    bool has_layout();

    SupplementMacro* get_macro(std::string const & id);
    void add_macro(SupplementMacro const * macro);

    SupplementLayout* get_layout();
    void set_layout(SupplementLayout * layout);

    SupplementTerminal* get_terminal(std::string const & id);
    void add_terminal(SupplementTerminal const * terminal);

private:
    Utils::Logger* m_logger;
    std::unordered_map<std::string, SupplementMacro*> m_macros;
    std::unordered_map<std::string, SupplementTerminal*> m_terminals;
    SupplementLayout* m_layout;
};

} /* namespace Placer */

#endif /* SUPPLEMENT_HPP */
