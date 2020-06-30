//==================================================================
// Author       : Pointner Sebastian
// Company      : Johannes Kepler University
// Name         : SMT Macro Placer
// Workfile     : utils.cpp
//
// Date         : 07.January 2019
// Compiler     : gcc version 9.2.0 (GCC) 
// Copyright    : Johannes Kepler University
// Description  : Utility Class
//==================================================================
#include "utils.hpp"

using namespace Placer::Utils;


/**
 * @brief Split a string into token separated by a given delimiter
 * 
 * @param str: The string to tokenize
 * @param delimiters: The delimiter to split the string
 * @return std::vector< std::string >
 */
std::vector<std::string> Utils::tokenize(std::string const & str, std::string const & delimiters) 
{
    std::vector<std::string> tokens;
    // skip delimiters at beginning.
    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);

    // find first "non-delimiter".
    std::string::size_type pos = str.find_first_of(delimiters, lastPos);

    while (std::string::npos != pos || std::string::npos != lastPos) {
        // found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));

        // skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);

        // find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }

    return tokens;
}

/**
 * @brief Execute a Tool on the Host System
 * 
 * @param binary Name of the binary
 * @param args Command line arguments
 * @param output Path for the logfile
 * @param wait_for_termination: Wait until the subprocess has terminated
 * @return int
 */
int Utils::system_execute(std::string const & binary,
                          std::vector<std::string> & args,
                          std::string const & output,
                          bool wait_for_termination)
{
    int pid;
    return Utils::Utils::system_execute(binary,
                                        args,
                                        output,
                                        pid,
                                        wait_for_termination);
}

/**
 * @brief Execute a Tool on the Host System
 * 
 * @param binary Name of the binary
 * @param args Command line arguments
 * @param output Path for the logfile
 * @param pid: Pid of the current subprocess 
 * @param wait_for_termination: Wait until the subprocess has terminated.
 * @return int
 */
int Utils::system_execute(std::string const & binary,
                          std::vector<std::string> & args,
                          std::string const & output,
                          int & pid,
                          bool wait_for_termination)
{
    try {
        boost::process::ipstream pipe_stream;
        boost::filesystem::path bin_url(binary);
        
        if(binary.find("/") == std::string::npos){
            bin_url = boost::process::search_path(binary);
        }

        if(output.empty()){
            boost::process::child bin(bin_url, boost::process::args(args));
            pid = bin.id();
            
            if(wait_for_termination){
                bin.wait();
                return bin.exit_code();
            } else {
                bin.detach();
                return 0;
            }
        } else {
            std::ofstream log(output);
            std::string line;
            boost::process::child bin(bin_url,
                                      boost::process::args(args),
                                      boost::process::std_out > pipe_stream);
            pid = bin.id();

            while (pipe_stream && std::getline(pipe_stream, line) && !line.empty()){
                log << line << std::endl;
            }

            if(wait_for_termination){
                bin.wait();
                return bin.exit_code();
            } else {
                bin.detach();
            }

            log.close();

            return 0;
        }

    } catch (boost::process::process_error const & exp){
        throw std::runtime_error(exp.what());
    } catch (std::exception const & exp){
        throw std::runtime_error(exp.what());
    }
}

/**
 * @brief Return a string which will blink red in the bash shell
 * 
 * @param str: The string to blink red in the terminal
 */ 
std::string Utils::get_bash_string_blink_red(const std::string& str) 
{
    return "\033[31;5m" + str + "\033[m";
}

/**
 * @brief Return a string which will appear cyan in the bash shell
 * 
 * @param str: The string to be cyan in the terminal
 */ 
std::string Utils::get_bash_string_cyan(const std::string& str)
{
    return "\033[36m" + str + "\033[0m";
}

/**
 * @brief Return a string which will blink purple in the bash shell
 * 
 * @param str: The string to blink red in the terminal
 */ 
std::string Utils::get_bash_string_purple(const std::string& str) 
{
    return "\033[35m" + str + "\033[m";
}

/**
 * @brief Resolve Forest's BasePath (Installation Root)
 * 
 * @return std::string
 */
std::string Utils::get_base_path()
{
    std::string file_content;
    std::string home_folder = getenv("HOME");
    std::string path = home_folder + "/.smt_placer/config.txt";
    std::fstream in_file(path, std::ios::in);
    std::getline(in_file, file_content);
    in_file.close();

    return Utils::tokenize(file_content, "=")[1];
}

/**
 * @brief Return a string which will appear orange in the bash shell
 * 
 * @param str: The string to be orange in the terminal
 */ 
std::string Utils::get_bash_string_orange(const std::string& str)
{
    return "\033[33m" + str + "\033[0m";
}

/**
 * @brief Return a string which will appear red in the bash shell
 * 
 * @param str: The string to be red in the terminal
 */ 
std::string Utils::get_bash_string_red(const std::string& str)
{
    return "\033[31m" + str + "\033[0m";
}

/**
 * @brief Return a string which will blink purple in the bash shell
 * 
 * @param str: The string to blink red in the terminal
 */ 
std::string Utils::get_bash_string_blink_purple(const std::string& str) 
{
     return "\033[35;5m" + str + "\033[m";
}

/**
 * @brief Return a string which will appear green in the bash shell
 * 
 * @param str: The string to be green in the terminal
 */ 
std::string Utils::get_bash_string_green(const std::string& str)
{
    return "\033[32m" + str + "\033[0m";
}

/**
 * @brief Return a string which will appear blue in the bash shell
 * 
 * @param str: The string to be blue in the terminal
 */ 
std::string Utils::get_bash_string_blue(const std::string& str)
{
    return "\033[34m" + str + "\033[0m";
}

/**
 * @brief Strip Bash Color Token from given String
 * 
 * @param str String to Strip
 * @return std::string
 */
std::string Utils::strip_bash_color_token(std::string const & str)
{
    if (str[0] == '\033'){
        std::string inprogress = str;
        size_t end = inprogress.find_last_of("\033");
        inprogress = inprogress.substr(0, end);
        size_t from = inprogress.find_first_of("m");
        inprogress = inprogress.substr(from+1, inprogress.size() - from);

        return inprogress;

    } else {
        return str;
    }
}

/**
 * @brief Get the current time of execution
 * 
 * @return std::string
 */
std::string Utils::get_current_time()
{
    auto time_now = std::chrono::system_clock::now();
    std::time_t now = std::chrono::system_clock::to_time_t(time_now);

    return std::ctime(&now);
}

/**
 * @brief Get the username of the current execution
 * 
 * @return std::string
 */
std::string Utils::get_current_user()
{
    char buf[50];
    getlogin_r(buf, 50);

    return std::string(buf);
}

/**
 * @brief Get Plattform name
 * 
 * @return std::string
 */
std::string Utils::get_plattform()
{
    struct utsname name;
    uname(&name);

    return std::string(name.sysname) + " " + std::string(name.release) + " " + std::string(name.machine);
}
