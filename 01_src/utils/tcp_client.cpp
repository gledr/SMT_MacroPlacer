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
    m_socket->connect(boost::asio::ip::tcp::endpoint( boost::asio::ip::address::from_string("127.0.0.1"), 1234 ));
}

void TCPClient::send(std::string const & data)
{
    boost::asio::write(*m_socket,
                       boost::asio::buffer(data),
                       m_error );
}

std::string TCPClient::receive()
{
    boost::asio::read(*m_socket,
                      m_receive_buffer,
                      boost::asio::transfer_all(),
                      m_error);

    if( m_error && m_error != boost::asio::error::eof ) {
        std::cout << "receive failed: " << m_error.message() << std::endl;
    }

    return std::string(boost::asio::buffer_cast<const char*>(m_receive_buffer.data()));
}
