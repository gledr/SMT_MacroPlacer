//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : hlclient.cpp
//
// Date         : 18. September 2020
// Compiler     : gcc version 10.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Client for Heuristics Lab Backend
//==================================================================
#include "hlclient.hpp"

using namespace Placer;
using namespace Placer::Utils;

/**
 * @brief CTOR
 */
HLClient::HLClient()
{
    m_tcp_client = new TCPClient();
    m_proto = new PlacerProto::MacroCircuit();
    m_logger = Logger::getInstance();
}

/**
 * @brief DTOR
 */
HLClient::~HLClient()
{
    delete m_tcp_client; m_tcp_client = nullptr;
    delete m_proto; m_proto = nullptr;
    delete m_proto_layout; m_proto_layout = nullptr;
}

/**
 * @brief Establish Connection
 */
void HLClient::connect()
{
    m_tcp_client->connect();
}

/**
 * @brief Close Connection
 */
void HLClient::disconnect()
{
    this->configure_server(eTerminate);
    m_tcp_client->disconnect();
}

/**
 * @brief Set Macros for Serialize
 * 
 * @param macros 
 */
void HLClient::set_macros(std::vector<Macro*> const & macros)
{
    m_macros = macros;
}

/**
 * @brief Set Layour for Serialize
 * 
 * @param layout 
 */
void HLClient::set_layout(Layout* layout)
{
    nullpointer_check(layout);

    m_layout = layout;
}

/**
 * @brief Run Serialization
 */
void HLClient::serialize()
{
    m_logger->start_serialize();
    for (Macro* m: m_macros){
        PlacerProto::Macro* pm = m_proto->add_m();
        nullpointer_check(pm);

        pm->set_name(m->get_name());
        pm->set_id(m->get_id());
        pm->set_height(m->get_height_numeral());
        pm->set_width(m->get_width_numeral());
    }
    
    m_proto_layout = new PlacerProto::Layout();
    m_proto_layout->set_lx(0);
    m_proto_layout->set_ly(0);
    m_proto->set_allocated_l(m_proto_layout);
    
    m_logger->end_serialize();
}

/**
 * @brief Run Deserialization
 */
void HLClient::deserialize()
{
    for(size_t i = 0; i < m_proto->m_size(); ++i){
        PlacerProto::Macro m = m_proto->m(i);
        std::string id = m.id();
        Macro* macro = this->find_macro(id);
        nullpointer_check(macro);
        m_logger->place_macro(id, m.lx(), m.ly(), eNorth);
        macro->add_solution_lx(m.lx());
        macro->add_solution_ly(m.ly());
        macro->add_solution_orientation(static_cast<eOrientation>(m.orientation()));
    }

    PlacerProto::Layout l = m_proto->l();
    m_layout->set_solution_ux(l.ux());
    m_layout->set_solution_uy(l.uy());
}

/**
 * @brief Transmit Placement Problem
 */
void HLClient::transmit_problem()
{
    this->serialize();
    this->configure_server(eSetProblem);
    std::string data = m_proto->SerializeAsString();
    m_tcp_client->send(data);

    this->configure_server(eInit);
}

/**
 * @brief Set Backend Server State
 * 
 * @param mode 
 */
void HLClient::configure_server(eBackendMode const mode)
{
    switch (mode){
        case eInit:
            m_tcp_client->send(INIT_TAG);
            break;
        case eSetProblem:
            m_tcp_client->send(SET_PROBLEM_TAG);
            break;
        case eSolve:
            m_tcp_client->send(SOLVE_PROBLEM_TAG);
            break;
        case eGetSolution:
            m_tcp_client->send(GET_SOLUTION_TAG);
            break;
        case eTerminate:
            m_tcp_client->send(TERMINATE_SERVER_TAG);
            break;
        default:
            notsupported_check(std::to_string(mode));
    }
}

/**
 * @brief Invoke Heuristics Lab on Backend
 */
void HLClient::solve_problem()
{
    this->configure_server(eSolve);
    m_logger->invoke_heuristics_lab();
    this->configure_server(eInit);
}

/**
 * @brief Get Solution from Backend
 */
void HLClient::get_solution()
{
    this->configure_server(eGetSolution);
    std::string data = m_tcp_client->receive();
   
    this->configure_server(eInit);
    m_logger->start_serialize();
    if (! m_proto->ParseFromString(data)){
        throw PlacerException("Can not Parse Incoming Protobuf Stream");
    }
   
    m_logger->end_deserialize();
    this->deserialize();
}

/**
 * @brief Get access to Macro by ID
 * 
 * @param id Macro ID
 * @return Placer::Macro*
 */
Macro* HLClient::find_macro(std::string const & id)
{
    Macro* m = nullptr;

    for (Macro* itor: m_macros){
        if (id == itor->get_id()){
            m = itor;
            break;
        }
    }
    nullpointer_check(m);
    assertion_check (id == m->get_id());

    return m;
}

/**
 * @brief Get Access to the Macros
 * 
 * @return std::vector< Placer::Macro* >
 */
std::vector<Macro*> HLClient::get_macros()
{
    return m_macros;
}

/**
 * @brief Get Access to the Layout
 * 
 * @return Placer::Layout*
 */
Layout* HLClient::get_layout()
{
    return m_layout;
}

/**
 * @brief ...
 * 
 * @param filename p_filename:...
 */
void HLClient::export_as_file(std::string const & filename)
{
    std::cout << "Writing Protobuf Content to " << filename << std::endl;
    this->serialize();
    std::string data = m_proto->SerializeAsString();
    std::ofstream export_file(filename);
    export_file << data;
    export_file.close();
}

/**
 * @brief ...
 * 
 * @param filename p_filename:...
 */
void HLClient::read_from_file(std::string const & filename)
{
    notimplemented_check();
}

