#pragma once

#include <iomanip>
#include <string>
#include <chrono>
#include <iostream>

#include "../../external/oof/oof.h"


namespace spr {
class SprLog{
public:
    SprLog();
    ~SprLog();

     static void debug(std::string msg){
        std::cout << oof::fg_color(m_timeColor);
        std::cout << getTime();
        std::cout << oof::fg_color(m_debugColor);
        std::cout << " [DEBUG]: ";
        std::cout << oof::fg_color(m_textColor);
        std::cout << msg;
        std::cout << oof::reset_formatting() << std::endl;
    }

    static void info(std::string msg){
        std::cout << oof::fg_color(m_timeColor);
        std::cout << getTime();
        std::cout << oof::fg_color(m_infoColor);
        std::cout << " [INFO]:  ";
        std::cout << oof::fg_color(m_textColor);
        std::cout << msg;
        std::cout << oof::reset_formatting() << std::endl;
    }

    static void warn(std::string msg){
        std::cout << oof::fg_color(m_timeColor);
        std::cout << getTime();
        std::cout << oof::fg_color(m_warnColor);
        std::cout << " [WARN]:  ";
        std::cout << oof::fg_color(m_textColor);
        std::cout << msg;
        std::cout << oof::reset_formatting() << std::endl;
    }

    static void error(std::string msg){
        std::cout << oof::fg_color(m_timeColor);
        std::cout << getTime();
        std::cout << oof::fg_color(m_errorColor);
        std::cout << " [ERROR]: ";
        std::cout << oof::fg_color(m_textColor);
        std::cout << msg;
        std::cout << oof::reset_formatting() << std::endl;
    }

    static void fatal(std::string msg){
        std::cout << oof::fg_color(m_timeColor);
        std::cout << getTime();
        std::cout << oof::fg_color(m_fatalColor);
        std::cout << " [FATAL]: ";
        std::cout << msg;
        std::cout << oof::reset_formatting() << std::endl;
    }

private:
    friend class SprLog;

    static constexpr oof::color m_timeColor{0,153,76};
    static constexpr oof::color m_textColor{255,255,255};
    static constexpr oof::color m_debugColor{255,51,153};
    static constexpr oof::color m_infoColor{192,192,192};
    static constexpr oof::color m_warnColor{255,153,51};
    static constexpr oof::color m_errorColor{255,51,51};
    static constexpr oof::color m_fatalColor{255,51,51};
    
    static std::string getTime() {
        // Get the current time
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm tm = *std::localtime(&t);

        // Format the time as a string in the format "[HH:MM:SS]"
        std::stringstream ss;
        ss << "[" << std::put_time(&tm, "%H:%M:%S") << "]";
        return ss.str();
    }

    static std::string getDate() {
        // Get the current time
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm tm = *std::localtime(&t);

        // Format the date and time as a string in the format "MM-DD-YYYY [HH:MM:SS]"
        std::stringstream ss;
        ss << std::put_time(&tm, "%m-%d-%Y [%H:%M:%S]");
        return ss.str();
    }
};
}