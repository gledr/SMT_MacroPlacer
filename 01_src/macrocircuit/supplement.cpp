//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : supplement.cpp
//
// Date         : 09. January 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : LEF/DEF Supplement File
//==================================================================
#include "supplement.hpp"

using namespace Placer;
using namespace Placer::Utils;

Supplement::Supplement():
    Object()
{
    m_logger = Utils::Logger::getInstance();
}

Supplement::~Supplement()
{
    m_logger = nullptr;
    
    for(auto itor: m_macros){
        delete itor.second; itor.second = nullptr;
    }
}

bool Supplement::has_supplement()
{
    return m_macros.size() > 0;
}

void Supplement::read_supplement_file()
{
    if(this->get_supplement().empty()){
        return;
    } else {
        m_logger->supplement_file(this->get_supplement());
    }
    
    if(!boost::filesystem::exists(this->get_supplement())){
        assert (0 && "Can not find Supplement File!");
    }

    Json::Value root;
    std::ifstream supplement_file(this->get_supplement());
    supplement_file >> root;
    supplement_file.close();

    for(auto itor = root.begin(); itor != root.end(); ++itor){
        for(auto itor2 = itor->begin(); itor2 != itor->end(); ++itor2){
            SupplementMacro* macro = new SupplementMacro(itor2.key().asString());
            std::string macro_id = itor2.key().asString();
           
            for(auto itor3 = itor2->begin(); itor3 != itor2->end(); ++itor3){
                SupplementPin* pin = new SupplementPin(itor3.key().asString());
                std::string pin_name = itor3.key().asString();
                
                for(auto itor4 = itor3->begin(); itor4 != itor3->end(); ++itor4){
                    std::string property = itor4.key().asString();
                    std::string value    = itor4->asString();
                    
                    if(property == "frequency"){
                        pin->set_frequency(std::stoi(value));
                    } else if (property == "width"){
                        pin->set_bitwidth(std::stoi(value));
                    } else {
                        assert (0 && "Unknown JSON Key!");
                    }
                }
                macro->add_pin(pin);
            }
            m_macros[itor2.key().asString()] = macro;
        }
    }
}

SupplementMacro* Supplement::get_macro(const std::string& id)
{
    SupplementMacro* retval = m_macros[id];
    assert (retval != nullptr);
    
    return retval;
}

bool Supplement::has_macro(std::string const & id)
{
    return m_macros[id] != nullptr;
}
