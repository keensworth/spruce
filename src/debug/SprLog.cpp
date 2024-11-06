#include "SprLog.h"
#include "util/color/Gradient.h"
#include <initializer_list>

#define OOF_IMPL
#include "external/oof/oof.h"


namespace spr {

const std::hash<std::string_view> SprLog::hash;
const std::string SprLog::blank = std::string(80, ' ');
int SprLog::count = 0;


// --------------------------------------------------------- //
//         Log                                               //
// --------------------------------------------------------- //

void SprLog::log(std::initializer_list<LogMsg> msgs){
    logPrivate(msgs,{});
}

void SprLog::logPrivate(std::initializer_list<LogMsg> msgs, std::initializer_list<LogMsg> msgs2){
    uint32 allFlags = msg::NONE;
    uint32 lastFlags = msg::NONE;
    oof::color color{};
    color::rgb32 msgColor{};
    count++;
    // lambda to write one message, different flags
    // influence behaviour (in order of priority):
    //
    // - CARRIAGE  carriage return (clear line)
    // - RESET     force CR, return with no newline
    // - BOLD      format bold (this LogMsg only)
    // - UNDERLINE format underline (this LogMsg only)
    // - TIMED     log current time, continue to next msg
    // - NEWLINE   newline, continue to next msg
    // - HOLD      don't insert newline automatically
    auto processMsg = [&](const LogMsg& msg) -> std::tuple<bool,bool> {
        
        if (msg.colorId){
            if (msg.color.id >= color::VIRIDIS){
                msgColor = color::sample(Span<color::rgb32>(color::gradientMap[msg.color.id]), (hash(msg.msg) % 255)/255.f);
            } else {
                msgColor = color::idMap[msg.color.id];
            }
        } else {
            msgColor = msg.color.vec;
        }
        color = {msgColor.r, msgColor.g, msgColor.b};
        allFlags |= msg.flags;
        lastFlags = msg.flags;
        bool terminate = msg.flags & msg::TERMINATE;

        // clear line and reset cursor
        // (carriage return | reset line)
        if (msg.flags & msg::CARRIAGE || msg.flags & msg::RESET)
            std::cout << "\r" << SprLog::blank << "\r";
        if (msg.flags & msg::RESET)
            return {true, terminate};
        
        // set bold / underline
        if (msg.flags & msg::BOLD)
            std::cout << oof::bold(true);
        if (msg.flags & msg::UNDERLINE)
            std::cout << oof::underline(true);

        // write time
        if (msg.flags & msg::TIMED) {
            std::cout << oof::fg_color(color);
            std::cout << getTime();
            return {false, terminate};
        }

        // write supplied message
        std::cout << oof::fg_color(color);
        std::cout << msg.msg;
        std::cout << oof::reset_formatting();

        // force a newline
        // (if this is done for last
        //  message, we won't default
        //  to doing this afterwards)
        if (msg.flags & msg::NEWLINE)
            std::cout << std::endl;

        return {false, terminate};
    };

    // log messages from both lists
    for (const auto& msg : msgs) {
        auto [reset, terminate] = processMsg(msg);
        
        if (reset) {
            if (!terminate) { return; }
            if (terminate) { break; }
        }
    }
    for (const auto& msg : msgs2) {
        auto [reset, terminate] = processMsg(msg);

        if (reset) {
            if (!terminate) { return; }
            if (terminate) { break; }
        }
    }

    // flush existing writes, reset 
    // formatting for next log() call
    std::cout.flush();
    std::cout << oof::reset_formatting();

    // if the last msg didn't write NL/newline 
    // (and isn't asking us not to), write it
    if (!(lastFlags & msg::NEWLINE || allFlags & msg::HOLD))
        std::cout << std::endl;
    
    // call terminate (error / fatal)
    if (allFlags & msg::TERMINATE)
        std::terminate();
}


// --------------------------------------------------------- //
//         Tiered Log (1 string)                             //
// --------------------------------------------------------- //

void SprLog::debug(const std::string& msg){
    SprLog::log({
        { color::TIME, msg::TIMED },
        { " [DEBUG]:", color::DEBUG, msg::BOLD },
        { msg, color::TEXT }
    });
}

void SprLog::info(const std::string& msg){
    SprLog::log({
        { color::TIME, msg::TIMED },
        { " [INFO]: ", color::INFO, msg::BOLD },
        { msg, color::TEXT }
    });
}

void SprLog::warn(const std::string& msg){
    SprLog::log({
        { color::TIME, msg::TIMED },
        { " [WARN]: ", color::WARN, msg::BOLD },
        { msg, color::TEXT }
    });
}

void SprLog::error(const std::string& msg, bool terminate){
    SprLog::log({
        { color::TIME, msg::TIMED },
        { "[ERROR]: ", color::ERROR, msg::BOLD },
        { msg, color::TEXT }
    });

    if (terminate)
        std::terminate();
}

void SprLog::fatal(const std::string& msg){
    SprLog::log({
        { color::TIME, msg::TIMED },
        { "[FATAL]: ", color::FATAL, msg::BOLD },
        { msg, color::TEXT }
    });
    std::terminate();
}


// --------------------------------------------------------- //
//         Tiered Log (N messages)                           //
// --------------------------------------------------------- //

void SprLog::debug(std::initializer_list<LogMsg> msgs){
    SprLog::logPrivate({
        { color::TIME, msg::TIMED },
        { " [DEBUG]:", color::DEBUG, msg::BOLD }},
        msgs
    );
}

void SprLog::info(std::initializer_list<LogMsg> msgs){
    SprLog::logPrivate({
        { color::TIME, msg::TIMED },
        { " [INFO]: ", color::INFO, msg::BOLD }},
        msgs
    );
}

void SprLog::warn(std::initializer_list<LogMsg> msgs){
    SprLog::logPrivate(
        {{ color::TIME, msg::TIMED },
        { " [WARN]: ", color::WARN, msg::BOLD }},
        msgs
    );
}

void SprLog::error(std::initializer_list<LogMsg> msgs){
    SprLog::logPrivate({
        { color::TIME, msg::TIMED },
        { "[ERROR]: ", color::ERROR, msg::BOLD }},
        msgs
    );
}

void SprLog::fatal(std::initializer_list<LogMsg> msgs){
    SprLog::logPrivate({
        { color::TIME, msg::TIMED },
        { "[FATAL]: ", color::FATAL, msg::BOLD }},
        msgs
    );
    std::terminate();
}


// --------------------------------------------------------- //
//         Time/Date Helper                                  //
// --------------------------------------------------------- //

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