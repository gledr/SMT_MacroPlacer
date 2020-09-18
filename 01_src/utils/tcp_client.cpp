//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : tcp_client.cpp
//
// Date         : 14.September 2020
// Compiler     : gcc version 10.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : TCP/IP Client
//==================================================================
#include "tcp_client.hpp"

using namespace Placer::Utils;

TCPClient::TCPClient()
{
    m_io_service = new boost::asio::io_service();
    m_socket = new  boost::asio::ip::tcp::socket(*m_io_service);
}

TCPClient::~TCPClient()
{
    delete m_io_service; m_io_service = nullptr;
    delete m_socket; m_socket = nullptr;
}

void TCPClient::connect ()
{
    bool success = false;
    using namespace boost::asio::ip;

    do {
        try {
            m_socket->connect(tcp::endpoint(address::from_string(this->get_hl_backend_ip()), this->get_hl_backend_port()));
            success = true;
        } catch (boost::system::system_error const & exp){
            std::cout << "Could not connect (" << exp.what() << ")" << std::endl;
            std::cout << "Retrying in 5 sec..." << std::endl;
            sleep(5);
        }
    } while (!success);
   
}

void TCPClient::disconnect()
{
    m_socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, m_error);
    m_socket->close();
}

void TCPClient::send(std::string const & data)
{
    boost::asio::write(*m_socket,
                       boost::asio::buffer(this->add_delimiter(data)),
                       m_error );
}

std::string TCPClient::receive()
{
    boost::asio::streambuf receive_buffer;

    boost::asio::read_until(*m_socket,
                      receive_buffer,
                      DELIMITER,
                      m_error);

    if( m_error && m_error != boost::asio::error::eof ) {
        std::cout << "receive failed: " << m_error.message() << std::endl;
    }

    std::string retval(boost::asio::buffer_cast<const char*>(receive_buffer.data()));
    return this->strip_delimiter(retval);
}

std::string TCPClient::add_delimiter(std::string const & data)
{
    return data + DELIMITER;
}

std::string TCPClient::strip_delimiter(std::string const & data)
{
    return data.substr(0, data.size()-1);
}
