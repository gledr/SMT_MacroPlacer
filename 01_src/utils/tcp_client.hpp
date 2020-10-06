//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : tcp_client.hpp
//
// Date         : 14.September 2020
// Compiler     : gcc version 10.2.0 (GCC)
// Copyright    : Johannes Kepler University
// Description  : TCP/IP Client
//==================================================================
#ifndef TCP_CLIENT_HPP
#define TCP_CLIENT_HPP

#include <object.hpp>
#include <logger.hpp>
#include <exception.hpp>

#include <boost/asio.hpp>

namespace Placer::Utils {
    
constexpr char DELIMITER[] = "#$#";
constexpr size_t DELIMITER_DIGITS = 3;
    
/**
 * @class TCPClient
 * @brief Simple TCP/IP Client
 */
class TCPClient : public virtual Object {
public:
    TCPClient();

    ~TCPClient();

    void connect();

    void disconnect();

    void send(std::string const & data);

    std::string receive();

private:
    Logger* m_logger;
    std::string strip_delimiter(std::string const & data);
    std::string add_delimiter(std::string const & data);

    boost::asio::io_service* m_io_service;
    boost::asio::ip::tcp::socket* m_socket;
    boost::system::error_code m_error;
};

} /* namespace Placer::Utils */

#endif /* TCP_CLIENT_HPP */
