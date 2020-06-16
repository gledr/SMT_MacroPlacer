//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : placer.hpp
//
// Date         : 27. December 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Macro Placer Main Class
//==================================================================
#ifndef MACRO_PLACER_HPP
#define MACRO_PLACER_HPP

#include <exception>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include <z3++.h>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <object.hpp>
#include <cmdline_arguments.hpp>
#include <macrocircuit.hpp>
#include <logger.hpp>
#include <version.hpp>
#include <utils.hpp>
#include <timer.hpp>

namespace Placer {

class MacroPlacer: public virtual Object {
public:
    MacroPlacer(int const argc, char ** argv);
    
    virtual ~MacroPlacer();

    void init();

    void run();
    
    void post_process();

private:
    int m_argc;
    char** m_argv;
    
    boost::program_options::options_description* m_options_functions;
    boost::program_options::variables_map m_vm;

    MacroCircuit* m_mckt;
    Evaluate* m_eval;
    Utils::Logger* m_logger;
    Utils::Timer* m_timer;

    void bash_completion_script();
    void print_header();

    void read_configuration();
    void handle_configuration();

    size_t existing_results();
    void store_configuration();
};

} /* namespace Placer */

#endif /* MACRO_PLACER_HPP */
