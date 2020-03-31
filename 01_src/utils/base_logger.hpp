//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : baselogger.hpp
//
// Date         : 07.January 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Abstract Base Logger Class
//==================================================================
#ifndef BASELOGGER_HPP
#define BASELOGGER_HPP

#include <iostream>
#include <fstream>
#include <mutex>

namespace Placer::Utils {

enum LogSeverity {
    eDebug      = 0, ///< Debug Logging
    eInfo       = 1, ///< Info Logging
    eWarning    = 2, ///< Warning Logging
    eError      = 3, ///< Error Logging
    eFatal      = 4, ///< Fatal Logging
    eStdOut     = 5  ///< StdOut Logging
};

/**
 * @class LogStream
 * @brief Stream Class used for Logging
 */ 
class LogStream {
public:
    /**
     * @brief Constructor
     */
    LogStream()
    {
    }

    /**
     * @brief Destructor
     */
    virtual ~LogStream()
    {
    }

    /**
     * @brief Stream Operator for Logging
     * 
     * @param val Value to Log
     * @return Forest::Utils::LogStream&
     */
    template<typename T>
    LogStream& operator<< (T const & val)
    { 
        std::string level_id = this->log_level_to_string(p_severity_level);

        if(p_log_enabled) {
            if(this->log_info()){
                if(this->p_log_to_file){
                    std::ofstream out_file(p_working_directory + "/" + p_file_name, std::ios::app);
                    out_file << level_id << val << std::endl;
                    out_file.close();
                }
                
            } else if (this->log_debug() || this->log_warning()){
                if(this->p_log_to_shell){
                    std::cout << level_id << val << std::endl;
                } 
                if(this->p_log_to_file){
                    std::ofstream out_file(p_working_directory + "/" + p_file_name, std::ios::app);
                    out_file << level_id << val << std::endl;
                    out_file.close();
                }
            }
        }

        if(this->log_error() || this->log_fatal()){
            std::cerr << level_id << val << std::endl;

            if(this->p_log_to_file){
                std::ofstream out_file(p_working_directory + "/" + p_file_name, std::ios::app);
                out_file << level_id << val << std::endl;
                out_file.close();
            }
        }
        
        if(this->log_stdout() && (!p_quiet)){
            std::cout << level_id << val << std::endl;

             if(this->p_log_to_file){
                std::ofstream out_file(p_working_directory + "/" + p_file_name, std::ios::app);
                out_file << level_id << val << std::endl;
                out_file.close();
            }
        }
        return *this;
    }

    /**
     * @brief Set Logging Severity
     * 
     * @param severity Severity Level
     */
    void set_log_level(LogSeverity const & severity)
    {
        this->p_severity_level = severity;
    }

    /**
     * @brief Enable Logging
     * 
     * @param val Enable/Disable Logging
     */
    void set_enabled(bool const val)
    {
        p_log_enabled = val;
    }

    /**
     * @brief Set Quiet Mode
     * 
     * @param val Enable/Disable Quiet Mode
     */
    void set_quiet(bool const val)
    {
        p_quiet = val;
    }

    /**
     * @brief Set Shell Logging
     * 
     * @param val Enable/Disable Logging
     */
    void set_dump_to_shell(bool const val)
    {
        p_log_to_shell = val;
    }

    /**
     * @brief Set Logfile logging
     * 
     * @param val Enable/Disable Logfile
     */
    void set_dump_to_file(bool const val)
    {
        p_log_to_file = val;
    }

    /**
     * @brief Set URL to store Logfile to
     * 
     * @param dir Directory URL
     */
    void set_working_directory(std::string const & dir)
    {
        p_working_directory = dir;
    }

    /**
     * @brief Set Filename for Logfile
     * 
     * @param file Filename
     */
    void set_file_name(std::string const file)
    {
        p_file_name = file;
    }

private:
    LogSeverity p_severity_level;

    bool p_log_enabled;
    bool p_quiet;
    bool p_log_to_shell;
    bool p_log_to_file;
    
    std::string p_working_directory;
    std::string p_file_name;
    
    
    /**
     * @brief Translate Loglevel to Readable String
     * 
     * @param level Loglevel
     * @return std::string
     */
    std::string log_level_to_string(LogSeverity const level)
    {
        std::string retval;
        
        switch (level) {
            case eInfo: 
                retval = "[Info]: ";
                break;

            case eDebug:
                retval = "[Debug]: ";
                break;

            case eWarning:
                retval = "[Warning]: ";
                break;

            case eError:
                retval = "[Error]: ";
                break;

            case eFatal:
                retval = "[Fatal]:";
                break;

            case eStdOut:
                retval = "[StdOut]:";
                break;
        }
        return retval;
    }
    
    /**
     * @brief Log Level Info Active
     * 
     * @return bool
     */
    inline bool log_info()
    {
        return p_severity_level == eInfo;
    }

    /**
     * @brief Log Level Debug Active
     * 
     * @return bool
     */
    inline bool log_debug()
    {
        return p_severity_level == eDebug;
    }

    /**
     * @brief Log Level Warning Active
     * 
     * @return bool
     */
    inline bool log_warning()
    {
        return p_severity_level == eWarning;
    }

    /**
     * @brief Log Level Error Active
     * 
     * @return bool
     */
    inline bool log_error()
    {
        return p_severity_level == eError;
    }

    /**
     * @brief Log Level Fatal Active 
     * 
     * @return bool
     */
    inline bool log_fatal()
    {
        return p_severity_level == eFatal;
    }

    /**
     * @brief Log Level StdOut Active
     * 
     * @return bool
     */
    inline bool log_stdout()
    {
        return p_severity_level == eStdOut;
    }
};

/**
 * @class BaseLogger
 * @brief BaseClass for all Forest Logger
 */
class BaseLogger {
protected:
    BaseLogger();

    virtual ~BaseLogger();

    static LogStream* p_log_stream;
    LogStream LOG(LogSeverity const & level);
};

}

#endif /* BASELOGGER_HPP */
