#include "util/Timer.h"
#include "debug/SprStats.h"

namespace spr::stats {

    double dt = 0.f;
    double dtAvg = 0.f;
    double low = 111.f;
    double high = 0.f;
    long fps = 0;
    long fpsAvg = 0;
    long frame = 0;
    long frameSlow = 0;

    double dtRunning = 0.f;
    double lowRunning = 111.f;
    double highRunning = 0.f;
    long fpsRunning = 0;
    long fpsAvgRunning = 0;
    long frameRunning = 0;

    long count = 0;

    Timer frameTimer;
    Timer updateTimer;

    void tick(){
        stats::frameTimer.start();
        stats::updateTimer.start();
    }

    void tock(){
        stats::count++;

        double dtNow = stats::frameTimer.elapsed();
        stats::frameTimer.stop();
        stats::dtRunning += dtNow;

        if (dtNow > stats::highRunning)
            stats::highRunning = dtNow;
        if (dtNow < stats::lowRunning)
            stats::lowRunning = dtNow;
        
        stats::fpsRunning = 1000.f / dtNow;
        stats::fpsAvgRunning += stats::fpsRunning;
        
        if (stats::updateTimer.elapsed() > 1000){
            stats::updateTimer.stop();

            stats::dt = dtNow;
            stats::low = stats::lowRunning;
            stats::high = stats::highRunning;
            stats::fps = stats::fpsRunning;
            stats::dtAvg = stats::dtRunning / (float)stats::count;
            stats::fpsAvg = stats::fpsAvgRunning / (float)stats::count;
            stats::frameSlow = stats::frameRunning;

            stats::dtRunning = 0.f;
            stats::lowRunning = 111.f;
            stats::highRunning = 0.f;
            stats::fpsRunning = 0;
            stats::fpsAvgRunning = 0;
            stats::count = 0;

            stats::updateTimer.start();
        }

        stats::frameTimer.start();
        stats::frame = stats::frameRunning;
        stats::frameRunning++;
    }


}