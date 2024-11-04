#pragma once

#include <chrono>

namespace spr {

typedef std::chrono::nanoseconds nanoseconds;
typedef std::chrono::microseconds microseconds;
typedef std::chrono::milliseconds milliseconds;
typedef std::chrono::seconds seconds;
typedef std::chrono::minutes minutes;
typedef std::chrono::hours hours;

typedef nanoseconds ns;
typedef microseconds us;
typedef milliseconds ms;
typedef seconds s;
typedef minutes m;
typedef hours h;

class Timer {
public:
    Timer() {}
    Timer(bool startOnConstruct) { if (startOnConstruct) start(); }
    ~Timer() {}

    void start(){
        if (m_running)
            return;

        m_start = std::chrono::high_resolution_clock::now();
        m_running = true;
    }

    void stop(){
        if (!m_running)
            return;

        m_stop = std::chrono::high_resolution_clock::now();
        m_running = false;
    }

    template <typename Duration = ms>
    double duration() {
        if (m_running)
            return 0.0;
        
        return std::chrono::duration<double>(std::chrono::duration_cast<Duration>(m_stop - m_start)).count();
    }

    template <typename Duration = ms>
    double elapsed() {
        auto end = m_running ? m_stop : std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double>(std::chrono::duration_cast<Duration>(end - m_start)).count();
    }

    bool running(){ return m_running; }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_stop;
    bool m_running = false;
};

}