#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>
#include <map>
#include <iostream>

namespace Placer::Utils {
    
class Timer {
public:
    Timer();
    
    ~Timer();
    
    void start_timer(std::string const & id);
    void stop_timer(std::string const & id);
    
    size_t read_timer_ms(std::string const & id);
    size_t read_timer_sec(std::string const & id);

private:
    std::map<std::string, std::chrono::time_point<std::chrono::system_clock>> m_begin;
    std::map<std::string, std::chrono::time_point<std::chrono::system_clock>> m_end;
};

}

#endif /* TIMER_HPP */
