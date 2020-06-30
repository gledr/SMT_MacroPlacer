//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : main.hpp
//
// Date         : 27. December 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Macro Placer Frontend
//==================================================================
#include <placer.hpp>
#include <z3++.h>

int main (int argc, char ** argv)
{
    Placer::MacroPlacer* placer = new Placer::MacroPlacer(argc, argv);
    placer->init();
    placer->run();
    placer->post_process();

    delete placer; placer = nullptr;

    return 0;
}
