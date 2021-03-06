//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : cell.cpp
//
// Date         : 22. December 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Standard Cell Component
//==================================================================
#include "cell.hpp"

using namespace Placer;

Cell::Cell(std::string const & name,
           std::string const & id):
    Component(),
    m_key(m_key_counter++)
{
    m_name = name;
    m_id = id;
}

Cell::~Cell()
{
}

bool Cell::operator==(Cell const & c)
{
    return (m_name == c.m_name) && (m_id == c.m_id);
}

void Cell::add_pin(Pin* _pin)
{
    nullpointer_check (_pin);
}

std::vector<Pin*> Cell::get_pins()
{
    return m_pins;
}

void Cell::dump(std::ostream& stream)
{
    stream << std::string(30, '$') << std::endl;
    stream << "Name (" << m_name << ") Id (" << m_id << " )" << std::endl;
    for(auto itor: m_pins){
        itor->dump(stream);
    }
    stream << std::string(30, '$') << std::endl;
    stream << std::endl;
}

size_t Cell::get_area()
{
    notimplemented_check();
}

size_t Cell::get_key()
{
    return m_key;
}

bool Cell::is_free()
{
    notimplemented_check();
}
