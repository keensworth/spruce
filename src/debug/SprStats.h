#pragma once

#include "util/Timer.h"

namespace spr::stats {

    extern double dt;
    extern double dtAvg;
    extern double low;
    extern double high;
    extern long fps;
    extern long fpsAvg;
    extern long frame;
    extern long frameSlow;

    extern void tick();
    extern void tock();
}