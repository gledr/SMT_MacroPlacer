//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : supplementmacro.cpp
//
// Date         : 09. January 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Supplement Macro Instance
//==================================================================
#include "supplementmacro.hpp"

using namespace Placer;

static bool debug = true;

SupplementMacro::SupplementMacro(std::string const & macro_id):
    m_id(macro_id)
{
    debug && std::cout << "[Info]: New Supplement Macro: " 
                       << macro_id << std::endl;
}

SupplementMacro::~SupplementMacro()
{
    for(auto itor: m_pins){
        delete itor.second; itor.second = nullptr;
    }
}

void SupplementMacro::add_pin(SupplementPin * pin)
{
    m_pins[pin->get_pinname()] = pin;
}

SupplementPin* SupplementMacro::get_pin(std::string const & name)
{
    return m_pins[name];
}

bool SupplementMacro::has_pins()
{
    return !m_pins.empty();
}

bool SupplementMacro::has_pin(std::string const & id)
{
    return m_pins[id] != nullptr;
}
