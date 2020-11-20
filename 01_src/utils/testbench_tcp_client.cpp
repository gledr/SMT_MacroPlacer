//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : testbench_tcp_client.hpp
//
// Date         : 15.September 2020
// Compiler     : gcc version 10.2.0 (GCC)
// Copyright    : Johannes Kepler University
// Description  : TCP/IP Client Testbench
//==================================================================
#include <iostream>

#include "tcp_client.hpp"

using namespace Placer::Utils;

int main () {
    TCPClient * client = new TCPClient();
    std::string package = "Hello World!";
    
    std::cout << "Trying to connect to Server..." << std::endl;
    client->connect();
    std::cout << "Connection Established :)" << std::endl;
    client->send(package);
    std::cout << "Send Dummy Package (" << package << ") to Server" << std::endl;
    std::string answer = client->receive();
    std::cout << "Received Answer from Server (" << answer << ")" << std::endl;
    client->disconnect();
    delete client; client = nullptr;

    return 0;
}
