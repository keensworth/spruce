#pragma once

#include <iomanip>
#include <string>
#include <chrono>
#include <iostream>

namespace oof {
    struct color;
}

namespace spr {

template<typename T>
concept arithmetic = std::integral<T> or std::floating_point<T>;

class SprLog{
public:
    SprLog() = default;
    ~SprLog() = default;

    static void debug(std::string msg);
    static void info(std::string msg);
    static void warn(std::string msg);
    static void error(std::string msg, bool terminate = true);
    static void fatal(std::string msg);

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
    static void debugHeader(std::string& msg);
    static void infoHeader(std::string& msg);
    static void warnHeader(std::string& msg);
    static void errorHeader(std::string& msg);
    static void fatalHeader(std::string& msg);
    static void resetFormatting();
};

template<arithmetic T>
void SprLog::debug(std::string msg, T arg){
    debugHeader(msg);
    std::cout << arg;
    resetFormatting();
}

template<arithmetic T>
void SprLog::info(std::string msg, T arg){
    infoHeader(msg);
    std::cout << arg;
    resetFormatting();
}

template<arithmetic T>
void SprLog::warn(std::string msg, T arg){
    warnHeader(msg);
    std::cout << arg;
    resetFormatting();
}

template<arithmetic T>
void SprLog::error(std::string msg, T arg){
    errorHeader(msg);
    std::cout << arg;
    resetFormatting();
    std::terminate();
}

template<arithmetic T>
void SprLog::fatal(std::string msg, T arg){
    fatalHeader(msg);
    std::cout << arg;
    resetFormatting();
    std::terminate();
}
}