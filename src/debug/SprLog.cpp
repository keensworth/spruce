#include "SprLog.h"

#define OOF_IMPL
#include "../../external/oof/oof.h"

namespace spr {

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

void SprLog::resetFormatting(){
    std::cout << oof::reset_formatting() << std::endl;
}

void SprLog::debug(std::string msg){
    std::cout << oof::fg_color({0,153,76});
    std::cout << getTime();
    std::cout << oof::fg_color({255,51,153});
    std::cout << " [DEBUG]: ";
    std::cout << oof::fg_color({255,255,255});
    std::cout << msg;
    std::cout << oof::reset_formatting() << std::endl;
}

void SprLog::info(std::string msg){
    std::cout << oof::fg_color({0,153,76});
    std::cout << getTime();
    std::cout << oof::fg_color({192,192,192});
    std::cout << " [INFO]:  ";
    std::cout << oof::fg_color({255,255,255});
    std::cout << msg;
    std::cout << oof::reset_formatting() << std::endl;
}

void SprLog::warn(std::string msg){
    std::cout << oof::fg_color({0,153,76});
    std::cout << getTime();
    std::cout << oof::fg_color({255,153,51});
    std::cout << " [WARN]:  ";
    std::cout << oof::fg_color({255,255,255});
    std::cout << msg;
    std::cout << oof::reset_formatting() << std::endl;
}

void SprLog::error(std::string msg, bool terminate){
    std::cout << oof::fg_color({0,153,76});
    std::cout << getTime();
    std::cout << oof::fg_color({255,51,51});
    std::cout << " [ERROR]: ";
    std::cout << oof::fg_color({255,255,255});
    std::cout << msg;
    std::cout << oof::reset_formatting() << std::endl;
    //throw SpruceErrorException();
    if (terminate)
        std::terminate();
}

void SprLog::fatal(std::string msg){
    std::cout << oof::fg_color({0,153,76});
    std::cout << getTime();
    std::cout << oof::fg_color({255,51,51});
    std::cout << " [FATAL]: ";
    std::cout << msg;
    std::cout << oof::reset_formatting() << std::endl;
    //throw new SpruceFatalException();
    std::terminate();
}

void SprLog::debugHeader(std::string& msg){
    std::cout << oof::fg_color({0,153,76});
    std::cout << getTime();
    std::cout << oof::fg_color({255,51,153});
    std::cout << " [DEBUG]: ";
    std::cout << oof::fg_color({255,255,255});
    std::cout << msg;
    std::cout << oof::fg_color({204,255,204});
}

void SprLog::infoHeader(std::string& msg){
    std::cout << oof::fg_color({0,153,76});
    std::cout << getTime();
    std::cout << oof::fg_color({192,192,192});
    std::cout << " [INFO]:  ";
    std::cout << oof::fg_color({255,255,255});
    std::cout << msg;
    std::cout << oof::fg_color({204,255,204});
}

void SprLog::warnHeader(std::string& msg){
    std::cout << oof::fg_color({0,153,76});
    std::cout << getTime();
    std::cout << oof::fg_color({255,153,51});
    std::cout << " [WARN]:  ";
    std::cout << oof::fg_color({255,255,255});
    std::cout << msg;
    std::cout << oof::fg_color({204,255,204});
}

void SprLog::errorHeader(std::string& msg){
    std::cout << oof::fg_color({0,153,76});
    std::cout << getTime();
    std::cout << oof::fg_color({255,51,51});
    std::cout << " [ERROR]: ";
    std::cout << oof::fg_color({255,255,255});
    std::cout << msg;
    std::cout << oof::fg_color({204,255,204});
}

void SprLog::fatalHeader(std::string& msg){
    std::cout << oof::fg_color({0,153,76});
    std::cout << getTime();
    std::cout << oof::fg_color({255,51,51});
    std::cout << " [FATAL]: ";
    std::cout << msg;
    std::cout << oof::fg_color({204,255,204});
}

}