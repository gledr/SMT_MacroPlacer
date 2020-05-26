//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : supplementmacro.hpp
//
// Date         : 09. January 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Supplement Macro Instance
//==================================================================
#ifndef SUPPLEMENTMACRO_HPP
#define SUPPLEMENTMACRO_HPP

#include <string>
#include <map>
#include <logger.hpp>

#include <supplementpin.hpp>

namespace Placer {

class SupplementMacro {
public:
    SupplementMacro(std::string const & macro_id);

    virtual ~SupplementMacro();

    bool has_pins();
    bool has_pin(std::string const & id);
    void add_pin(SupplementPin * pin);
    SupplementPin* get_pin(std::string const & name);
private:
    Utils::Logger* m_logger;
    std::string m_id;
    std::map<std::string, SupplementPin*> m_pins;
};

} /* namespace Placer */

#endif /* SUPPLEMENTMACRO_HPP */
