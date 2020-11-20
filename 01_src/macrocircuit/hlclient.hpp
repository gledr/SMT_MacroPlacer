//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : hlclient.hpp
//
// Date         : 18. September 2020
// Compiler     : gcc version 10.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Client for Heuristics Lab Backend
//==================================================================
#ifndef HLCLIENT_HPP
#define HLCLIENT_HPP

#include <object.hpp>
#include <exception.hpp>
#include <tcp_client.hpp>
#include <smt_placer.pb.h>
#include <logger.hpp>
#include <components.hpp>

namespace Placer {

enum eBackendMode {eInit, eSetProblem, eSolve, eGetSolution, eTerminate};

constexpr char INIT_TAG[]               = "#**#INIT#**#";
constexpr char SET_PROBLEM_TAG[]        = "#**#SETPROBLEM#**#";
constexpr char SOLVE_PROBLEM_TAG[]      = "#**#SOLVEPROBLEM#**#";
constexpr char GET_SOLUTION_TAG[]       = "#**#GETSOLUTION#**#";
constexpr char TERMINATE_SERVER_TAG[]   = "#**#TERMINATESERVER#**#";

/**
 * @class HLClient
 * @brief Client for Heuristics Lab Backend
 */
class HLClient: public virtual Object
{
public:

    HLClient();

    ~HLClient();

    void connect();

    void disconnect();

    void serialize();

    void deserialize();

    void set_macros(std::vector<Macro*> const & macros);

    void set_layout(Layout* layout);

    void configure_server(eBackendMode const mode);

    void transmit_problem();

    void get_solution();

    void solve_problem();

    std::vector<Macro*> get_macros();
    Layout* get_layout();

private:
    Utils::TCPClient* m_tcp_client;
    Utils::Logger* m_logger;
    
    std::vector<Macro*> m_macros;
    Layout* m_layout;
    PlacerProto::Layout* m_proto_layout;
    Macro* find_macro(std::string const & id);

    PlacerProto::MacroCircuit* m_proto;
};

} /* namespace Placer */

#endif // HLCLIENT_HPP
