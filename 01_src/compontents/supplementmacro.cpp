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
using namespace Placer::Utils;

SupplementMacro::SupplementMacro(std::string const & macro_id):
    m_id(macro_id)
{
    m_logger = Logger::getInstance();
    m_logger->add_supplement_macro(macro_id);
}

SupplementMacro::~SupplementMacro()
{
    for(auto itor: m_pins){
        delete itor.second; itor.second = nullptr;
    }
    m_logger = nullptr;
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
