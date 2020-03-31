#include "timer.hpp"

using namespace Placer::Utils;


Timer::Timer()
{
}

Timer::~Timer()
{
}

void Timer::start_timer(std::string const & id)
{
    m_begin[id] = std::chrono::system_clock::now();
}

void Timer::stop_timer(std::string const & id)
{
    m_end[id] = std::chrono::system_clock::now();
}

size_t Timer::read_timer_ms(std::string const & id)
{
    auto start = m_begin[id];
    auto end   = m_end[id];
    auto duration = end -start;
    
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

size_t Timer::read_timer_sec(std::string const & id)
{
    auto start = m_begin[id];
    auto end   = m_end[id];
    auto duration = end - start;
    size_t val = std::chrono::duration_cast<std::chrono::seconds>(duration).count();

    return val;
}
