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

/**
 * @brief Constructor
 */
TCPClient::TCPClient()
{
    m_io_service = new boost::asio::io_service();
    m_socket = new  boost::asio::ip::tcp::socket(*m_io_service);
    m_logger = Logger::getInstance();
}

/**
 * @brief Destructor
 */
TCPClient::~TCPClient()
{
    delete m_io_service; m_io_service = nullptr;
    delete m_socket; m_socket = nullptr;
}

/**
 * @brief Establish Connection to Server
 */
void TCPClient::connect ()
{
    bool success = false;
    using namespace boost::asio::ip;

    do {
        try {
            m_logger->connect_to_backend(this->get_hl_backend_ip(), this->get_hl_backend_port());
            m_socket->connect(tcp::endpoint(address::from_string(this->get_hl_backend_ip()), this->get_hl_backend_port()));
            success = true;
            m_logger->connected();
        } catch (boost::system::system_error const & exp){
            m_logger->connection_retry();
            sleep(5);
        }
    } while (!success);
   
}

/**
 * @brief Close Connection to Server
 */
void TCPClient::disconnect()
{
    m_socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, m_error);
    m_socket->close();
}

/**
 * @brief Send Data to Server
 * 
 * @param data Data to send
 */
void TCPClient::send(std::string const & data)
{
    m_logger->send(data.size());
    
    std::string sendme = this->add_delimiter(data); 
    m_socket->send(boost::asio::buffer(sendme, sendme.size()));
}

/**
 * @brief Receive Data from Server
 * 
 * @return std::string
 */
std::string TCPClient::receive()
{
    boost::asio::streambuf receive_buffer;

    // Use a Delimiter to identify end of stream
    size_t bytes = boost::asio::read_until(*m_socket,
                      receive_buffer,
                      DELIMITER,
                      m_error);

    m_logger->receive(bytes);
    if( m_error && m_error != boost::asio::error::eof) {
        throw PlacerException(m_error.message());
    }
     std::string retval{
        buffers_begin(receive_buffer.data()),
        buffers_begin(receive_buffer.data()) + bytes - DELIMITER_DIGITS};
          
    receive_buffer.consume(bytes);
  
    if (retval.size() + DELIMITER_DIGITS != bytes){
            throw PlacerException("TCP/IP Receive Data Failed!");
    }
   
    return retval;
}

/**
 * @brief Add Delimiter to Data
 * 
 * @param data Data to add delimiter to
 * @return std::string
 */
std::string TCPClient::add_delimiter(std::string const & data)
{
    return data + DELIMITER;
}

/**
 * @brief Strip Delimiter from data
 * 
 * @param data Data to strip delimiter from
 * @return std::string
 */
std::string TCPClient::strip_delimiter(std::string const & data)
{
    return data.substr(0, data.size() - DELIMITER_DIGITS);
}
