#include "SprLog.h"

#define OOF_IMPL
#include "../../external/oof/oof.h"
#include "util/Span.h"

namespace spr {

const std::string SprLog::blank = std::string(80, ' ');

void SprLog::log(std::initializer_list<LogMsg> msgs){
    int allFlags = NONE;
    int lastFlags = NONE;

    for (auto& msg : msgs){
        oof::color color = {msg.color.x, msg.color.y, msg.color.z};
        allFlags |= msg.flags;
        lastFlags = msg.flags;

        if (msg.flags & TIMED){
            std::cout << oof::fg_color(color);
            std::cout << getTime();
            continue;
        }

        if (msg.flags & CR || msg.flags & RESET){
            std::cout << "\r" << SprLog::blank << "\r";
            std::cout.flush();
        }

        if (msg.flags & RESET)
            return;
        
        if (msg.flags & BOLD)
            std::cout << oof::bold(true);
        if (msg.flags & UNDERLINE)
            std::cout << oof::underline(true);

        std::cout << oof::fg_color(color);
        std::cout << msg.str;
        std::cout << oof::reset_formatting();
        
        if (msg.flags & NL)
            std::cout << std::endl;
    }
    std::cout.flush();
    std::cout << oof::reset_formatting();

    if (!(allFlags & NL) && (allFlags & HOLD))
        return;

    if (lastFlags & NL)
        return;
    
    std::cout << std::endl;
}

void SprLog::debug(std::string msg){
    SprLog::log({
        { .color = color::time, .flags = TIMED },
        { .str = " [DEBUG]: ", .color = color::debug },
        { .str = msg, .color = color::white }
    });
}

void SprLog::info(std::string msg){
    SprLog::log({
        { .color = color::time, .flags = TIMED },
        { .str = " [INFO]:  ", .color = color::info },
        { .str = msg, .color = color::white }
    });
}

void SprLog::warn(std::string msg){
    SprLog::log({
        { .color = color::time, .flags = TIMED },
        { .str = " [WARN]:  ", .color = color::warn },
        { .str = msg, .color = color::white }
    });
}

void SprLog::error(std::string msg, bool terminate){
    SprLog::log({
        { .color = color::time, .flags = TIMED },
        { .str = " [ERROR]: ", .color = color::error },
        { .str = msg, .color = color::white }
    });

    if (terminate)
        std::terminate();
}

void SprLog::fatal(std::string msg){
    SprLog::log({
        { .color = color::time, .flags = TIMED },
        { .str = " [FATAL]: ", .color = color::fatal },
        { .str = msg, .color = color::white }
    });
    std::terminate();
}

std::string SprLog::getTime() {
    // Get the current time
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);

    // Format the time as a string in the format "[HH:MM:SS]"
    std::stringstream ss;
    ss << "[" << std::put_time(&tm, "%H:%M:%S") << "]";
    return ss.str();
}

std::string SprLog::getDate() {
    // Get the current time
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);

    // Format the date and time as a string in the format "MM-DD-YYYY [HH:MM:SS]"
    std::stringstream ss;
    ss << std::put_time(&tm, "%m-%d-%Y [%H:%M:%S]");
    return ss.str();
}

}