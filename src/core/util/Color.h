#pragma once
#include "spruce_core.h"

namespace spr::color {

typedef uvec3 rgb32;

static const rgb32 white = {255, 255, 255};
static const rgb32 black = {  0,   0,   0};
static const rgb32 debug = {255,  51, 153};
static const rgb32 info  = {192, 192, 192};
static const rgb32 warn  = {255, 153,  51};
static const rgb32 error = {255,  51,  51};
static const rgb32 fatal = {255,  51,  51};
static const rgb32 value = {212, 224, 155};
static const rgb32 time  = {  0, 153,  76};
static const rgb32 label = {153, 217, 140};

enum id {
    WHITE,
    BLACK,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    VALUE,
    TIME,
    LABEL
};

static const rgb32 idMap[10] = {
    white,
    black,
    debug,
    info,
    warn,
    error,
    fatal,
    value,
    time,
    label
};

}