//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : supplementlayout.hpp
//
// Date         : 14. May 2020
// Compiler     : gcc version 9.3.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Supplement Layout Instance
//==================================================================
#include "supplement.hpp"

using namespace Placer;
using namespace Placer::Utils;

Supplement::Supplement():
    Object()
{
    m_logger = Utils::Logger::getInstance();
    m_layout = nullptr;
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
    return (m_macros.size() > 0) || (m_layout != nullptr);
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
            std::string type = itor2.key().asString();
            
            if (type == "macro"){
                for(auto itor3 = itor2->begin(); itor3 != itor2->end(); ++itor3){
                SupplementMacro* macro = new SupplementMacro(itor3.key().asString());
                std::string macro_id = itor3.key().asString();

                    for(auto itor4 = itor3->begin(); itor4 != itor3->end(); ++itor4){
                        SupplementPin* pin = new SupplementPin(itor4.key().asString());
                        std::string pin_name = itor4.key().asString();
                        
                        for(auto itor5 = itor4->begin(); itor5 != itor4->end(); ++itor5){
                            std::string property = itor5.key().asString();
                            std::string value    = itor5->asString();
                            
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
                    m_macros[itor3.key().asString()] = macro;
                }
            } else if (type == "layout"){
                int lx = INT32_MIN;
                int ly = INT32_MIN;
                int ux = INT32_MIN;
                int uy = INT32_MIN;
                
                for(auto itor3 = itor2->begin(); itor3 != itor2->end(); ++itor3){
                    std::string property = itor3.key().asString();
                    
                    if (property == "lx"){
                        lx = itor3->asUInt();
                    } else if (property == "ly"){
                        ly = itor3->asUInt();
                    } else if (property == "ux"){
                        ux = itor3->asUInt();
                    } else if (property == "uy"){
                        uy = itor3->asUInt();
                    }
                }
                
                if (lx == INT32_MIN){
                    throw std::runtime_error("Missing lx entry in Supplement!");
                }
                if (ly == INT32_MIN){
                    throw std::runtime_error("Missing ly entry in Supplement!");
                }
                if (ux == INT32_MIN){
                    throw std::runtime_error("Missing ux entry in Supplement!");
                }
                if (uy == INT32_MIN){
                    throw std::runtime_error("Missing uy entry in Supplement!");
                }
                
                assert (m_layout == nullptr);
                m_layout = new SupplementLayout(lx, ly, ux, uy);
            }
        }
    }
}

SupplementMacro* Supplement::get_macro(std::string const & id)
{
    SupplementMacro* retval = m_macros[id];
    nullpointer_check (retval);

    return retval;
}

SupplementTerminal* Supplement::get_terminal(std::string const & id)
{
    SupplementTerminal* retval = m_terminals[id];
    nullpointer_check(retval);

    return retval;
}

SupplementLayout* Supplement::get_layout()
{
    return m_layout;
}

bool Supplement::has_macro(std::string const & id)
{
    return m_macros[id] != nullptr;
}

bool Supplement::has_terminal(std::string const & id)
{
    return m_terminals[id] != nullptr;
}

bool Supplement::has_layout()
{
    return m_layout != nullptr;
}
