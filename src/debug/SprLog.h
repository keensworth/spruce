#pragma once

#include <iomanip>
#include <string>
#include <chrono>
#include <iostream>
#include "util/Color.h"

namespace oof {
    struct color;
}

namespace spr {

template<typename T>
concept arithmetic = std::integral<T> or std::floating_point<T>;

enum FormatFlags {
    NONE = 0,
    UNDERLINE = 1,
    BOLD = 1 << 1,
    TIMED = 1 << 2,
    CR = 1 << 3,
    NL = 1 << 4,
    HOLD = 1 << 5,
    RESET = 1 << 6
};

struct LogMsg {
    std::string str;
    glm::uvec3 color;
    FormatFlags flags;
};

class SprLog{
public:
    SprLog() = default;
    ~SprLog() = default;

    static void debug(std::string msg);
    static void info(std::string msg);
    static void warn(std::string msg);
    static void error(std::string msg, bool terminate = true);
    static void fatal(std::string msg);

    static void log(std::initializer_list<LogMsg> msgs);

    template<arithmetic T>
    static void debug(std::string msg, T arg);
    template<arithmetic T>
    static void info(std::string msg, T arg);
    template<arithmetic T>
    static void warn(std::string msg, T arg);
    template<arithmetic T>
    static void error(std::string msg, T arg);
    template<arithmetic T>
    static void fatal(std::string msg, T arg);

private:
    static std::string getTime();
    static std::string getDate();

public:
    static const std::string blank;
};

template<arithmetic T>
void SprLog::debug(std::string msg, T arg){
    SprLog::log({
        { .color = color::time, .flags = TIMED },
        { .str = " [DEBUG]: ", .color = color::debug },
        { .str = msg, .color = color::white },
        { .str = std::to_string(arg), .color = color::value }
    });
}

template<arithmetic T>
void SprLog::info(std::string msg, T arg){
    SprLog::log({
        { .color = color::time, .flags = TIMED },
        { .str = " [INFO]:  ", .color = color::info },
        { .str = msg, .color = color::white },
        { .str = std::to_string(arg), .color = color::value }
    });
}

template<arithmetic T>
void SprLog::warn(std::string msg, T arg){
    SprLog::log({
        { .color = color::time, .flags = TIMED },
        { .str = " [WARN]:  ", .color = color::warn },
        { .str = msg, .color = color::white },
        { .str = std::to_string(arg), .color = color::value }
    });
}

template<arithmetic T>
void SprLog::error(std::string msg, T arg){
    SprLog::log({
        { .color = color::time, .flags = TIMED },
        { .str = " [ERROR]: ", .color = color::error },
        { .str = msg, .color = color::white },
        { .str = std::to_string(arg), .color = color::value }
    });
    std::terminate();
}

template<arithmetic T>
void SprLog::fatal(std::string msg, T arg){
    SprLog::log({
        { .color = color::time, .flags = TIMED },
        { .str = " [FATAL]: ", .color = color::fatal },
        { .str = msg, .color = color::white },
        { .str = std::to_string(arg), .color = color::value }
    });
    std::terminate();
}
}