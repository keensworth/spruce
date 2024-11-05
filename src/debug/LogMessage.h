#include <string>
#include <charconv>
#include "util/Color.h"


namespace spr {

template<typename T>
concept arithmetic = std::integral<T> or std::floating_point<T>;

template<typename T>
concept stringlike = std::is_convertible_v<T, std::string_view>
                  or std::is_convertible_v<T, std::string> or std::integral<T> or std::floating_point<T>;

namespace msg{
enum flags {
    NONE      = 0,
    UNDERLINE = 1 << 0,
    BOLD      = 1 << 1,
    TIMED     = 1 << 2,
    CARRIAGE  = 1 << 3,
    NEWLINE   = 1 << 4,
    HOLD      = 1 << 5,
    RESET     = 1 << 6,
    VALUE     = 1 << 7,
    TERMINATE = 1 << 8
};
}


struct LogMsg {
    // std::string_view as input
    LogMsg(std::string_view message, color::id color = color::WHITE, msg::flags flags = msg::NONE)
        : msg(message), color{.id = color}, flags(flags), colorId(true) {}

    LogMsg(std::string_view message, const color::rgb32& color, msg::flags flags = msg::NONE)
        : msg(message), color{.vec = color}, flags(flags), colorId(false) {}

    // T (numeric) as input
    template <arithmetic T>
    LogMsg(T value, color::id color = color::VALUE, msg::flags flags = msg::VALUE)
        : msg{}, data{}, color{.id = color}, flags(flags), colorId(true) {
        auto [ptr, ec] = std::to_chars(data, data + sizeof(data), value);
        msg = {data, (size_t)(ptr - data)};
    }

    template <arithmetic T>
    LogMsg(T value, const color::rgb32& color, msg::flags flags = msg::VALUE)
        : msg{}, data{}, color{.vec = color}, flags(flags), colorId(false) {
        auto [ptr, ec] = std::to_chars(data, data + sizeof(data), value);
        msg = {data, (size_t)(ptr - data)};
    }
    
    // formatting, blank message
    LogMsg(color::id color, msg::flags flags = msg::VALUE)
        : msg{}, color{.id = color}, flags(flags), colorId(true) {}

    LogMsg(const color::rgb32& color, msg::flags flags = msg::VALUE)
        : msg{}, color{.vec = color}, flags(flags), colorId(false) {}

    LogMsg(msg::flags flags)
        : msg{}, color{.id = color::WHITE},flags(flags), colorId(true) {}


    union Color {
        color::rgb32 vec;
        color::id id;
    };

    std::string_view msg;
    char data[16];

    Color color;
    msg::flags flags;
    bool colorId;
    int pad0;
    int pad1;
};

}