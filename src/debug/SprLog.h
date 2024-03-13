#pragma once

#include <iomanip>
#include <string>
#include <chrono>
#include <iostream>


namespace oof {
    struct color;
}

namespace spr {

class SpruceErrorException : public std::exception {
public:
    char * what () {
        return (char *)"spruce ERROR";
    }
private:
    //friend class SprLog;
    SpruceErrorException(){}
    ~SpruceErrorException(){}
};

class SpruceFatalException : public std::exception {
public:
    char * what () {
        return (char *)"spruce FATAL";
    }
private:
    //friend class SprLog;
    SpruceFatalException(){}
    ~SpruceFatalException(){}
};

class SprLog{
public:
    SprLog();
    ~SprLog();

    static void debug(std::string msg);
    static void info(std::string msg);
    static void warn(std::string msg);
    static void error(std::string msg, bool terminate = true);
    static void fatal(std::string msg);

    
    static void debug(std::string msg, uint32_t arg);
    
    static void info(std::string msg, uint32_t arg);
    
    static void warn(std::string msg, uint32_t arg);
    
    static void error(std::string msg, uint32_t arg);
    
    static void fatal(std::string msg, uint32_t arg);

    

private:
    static std::string getTime();
    static std::string getDate();
};
}