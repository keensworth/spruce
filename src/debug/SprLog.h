#pragma once

#include <iomanip>
#include <string>
#include <chrono>
#include <iostream>
#include "LogMessage.h"


namespace spr {

class SprLog{
public:
    SprLog() = default;
    ~SprLog() = default;

    static void debug(const std::string& msg);
    static void info(const std::string& msg);
    static void warn(const std::string& msg);
    static void error(const std::string& msg, bool terminate = true);
    static void fatal(const std::string& msg);

    static void debug(std::initializer_list<LogMsg> msgs);
    static void info(std::initializer_list<LogMsg> msgs);
    static void warn(std::initializer_list<LogMsg> msgs);
    static void error(std::initializer_list<LogMsg> msgs);
    static void fatal(std::initializer_list<LogMsg> msgs);

    static void log(std::initializer_list<LogMsg> msgs);

    template<arithmetic T>
    static void debug(const std::string& msg, T arg);
    template<arithmetic T>
    static void info(const std::string& msg, T arg);
    template<arithmetic T>
    static void warn(const std::string& msg, T arg);
    template<arithmetic T>
    static void error(const std::string& msg, T arg);
    template<arithmetic T>
    static void fatal(const std::string& msg, T arg);

private:
    static void logPrivate(std::initializer_list<LogMsg> msgs, std::initializer_list<LogMsg> msgs2);
    static std::string getTime();
    static std::string getDate();
    static const std::hash<std::string_view> hash;
    static int count;

public:
    static const std::string blank;
};

template<arithmetic T>
void SprLog::debug(const std::string& msg, T arg){
    SprLog::log({
        { color::TIME, msg::TIMED },
        { " [DEBUG]:", color::DEBUG, msg::BOLD },
        { msg, color::TEXT },
        { arg, color::VALUE }
    });
}

template<arithmetic T>
void SprLog::info(const std::string& msg, T arg){
    SprLog::log({
        { color::TIME, msg::TIMED },
        { " [INFO]: ", color::INFO, msg::BOLD },
        { msg, color::TEXT },
        { arg, color::VALUE }
    });
}

template<arithmetic T>
void SprLog::warn(const std::string& msg, T arg){
    SprLog::log({
        { color::TIME, msg::TIMED },
        { " [WARN]: ", color::WARN, msg::BOLD },
        { msg, color::TEXT },
        { arg, color::VALUE }
    });
}

template<arithmetic T>
void SprLog::error(const std::string& msg, T arg){
    SprLog::log({
        { color::TIME, msg::TIMED },
        { "[ERROR]: ", color::ERROR, msg::BOLD },
        { msg, color::TEXT },
        { arg, color::VALUE }
    });
    std::terminate();
}

template<arithmetic T>
void SprLog::fatal(const std::string& msg, T arg){
    SprLog::log({
        { color::TIME, msg::TIMED },
        { "[FATAL]: ", color::FATAL, msg::BOLD },
        { msg, color::TEXT },
        { arg, color::VALUE }
    });
    std::terminate();
}
}