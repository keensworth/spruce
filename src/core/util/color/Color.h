#pragma once
#include "Palette.h"

namespace spr::color {

static const rgb32 white = rgb(255,255,255);
static const rgb32 black = rgb(0,0,0);
static const rgb32 text  = rgb(240, 234, 224);
static const rgb32 debug = rgb(247,37,133);
static const rgb32 info  = rgb(139,255,75);
static const rgb32 warn  = rgb(238,155,0);
static const rgb32 error = rgb(249,65,68);
static const rgb32 fatal = rgb(249,65,68);
static const rgb32 value = rgb(208,98,87);
static const rgb32 time  = rgb(130,142,130);
static const rgb32 chars = rgb(233,196,106);

enum id {
    WHITE,
    BLACK,
    TEXT,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    VALUE,
    TIME,
    LABEL,
    CHARS,
    MAGMA,
    VIRIDIS,
    GRADIENT0,
    GRADIENT1,
    GRADIENT2,
    GRADIENT3,
    GRADIENT4,
    GRADIENT5,
    GRADIENT6,
    GRADIENT7,
    GRADIENT8,
    GRADIENT9,
    GRADIENT10,
    GRADIENT11,
    GRADIENT12,
    GRADIENT13,
    GRADIENT14,
    GRADIENT15,
    GRADIENT16,
    GRADIENT17,
    GRADIENT18,
    GRADIENT19,
    GRADIENT20,
    TURBO
};

static const rgb32 idMap[] = {
    white,
    black,
    text,
    debug,
    info,
    warn,
    error,
    fatal,
    value,
    time,
    black,
    chars,
    magma[0],
    viridis[0],
    gradients0[0],
    gradient1[0],
    gradient2[0],
    gradient3[0],
    gradient4[0],
    gradient5[0],
    gradient6[0],
    gradient7[0],
    gradient8[0],
    gradient9[0],
    gradient10[0],
    gradient11[0],
    gradient12[0],
    gradient13[0],
    gradient14[0],
    gradient15[0],
    gradient16[0],
    gradient17[0],
    gradient18[0],
    gradient19[0],
    gradient20[0],
    turbo[0],
};

}