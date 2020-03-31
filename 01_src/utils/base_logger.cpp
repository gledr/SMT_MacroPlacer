//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : baselogger.cpp
//
// Date         : 07.January 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Abstract Base Logger Class
//==================================================================
#include "base_logger.hpp"

using namespace Placer::Utils;

LogStream* BaseLogger::p_log_stream = nullptr;

/**
 * @brief Constructor
 */
BaseLogger::BaseLogger()
{
    p_log_stream = new LogStream();
}

/**
 * @brief Destructor
 */
BaseLogger::~BaseLogger()
{
    delete p_log_stream; p_log_stream = nullptr;
}

/**
 * @brief Logging Method
 * 
 * @param level Log Leve to be used.
 * @return Forest::Utils::LogStream
 */
LogStream BaseLogger::LOG(LogSeverity const & level)
{
    p_log_stream->set_log_level(level);
    return *(BaseLogger::p_log_stream);
}
