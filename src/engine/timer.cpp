/*
 * The game's timers: ticks of the performance counter (ReadTSC), converted with its frequency, g_cpuHz.
 */
#include "timer.h"
#include "sdw_classes.h"

#include "../sdk/win32.h"
#include "../sdk/crt.h"

double g_cpuHz;

/* g_cpuHz: the performance counter's frequency, which ReadTSC reads (below). The original measured its RDTSC against
 * QueryPerformanceCounter over a busy loop of sin(pow(i, 10)) calls; with the counter in place of RDTSC that measured
 * the counter against itself, and an optimising compiler drops the loop (its result is unused), leaving reads too close
 * together: a division by zero, which ARM64 answers with 0, made g_cpuHz several times too small and the game run that
 * much faster in Release builds. */
Timer::Timer()
{
    if (g_cpuHz == 0.0) {
        s64 frequency = 0;
        QueryPerformanceFrequency(&frequency);
        g_cpuHz = (double)frequency;
    }
    baseTsc = 0;
    elapsedTsc = 0;
    deltaTsc = 0;
    running = 0;
}

Timer::~Timer() {}

void Timer::Start()
{
    running = 1;
    baseTsc = ReadTSC();
    elapsedTsc = 0;
    deltaTsc = 0;
}

/* takes a last sample, then stops. */
void Timer::Stop()
{
    s64 previous = elapsedTsc;
    elapsedTsc = ReadTSC() - baseTsc;
    deltaTsc = elapsedTsc - previous;
    running = 0;
}

/* Time_Resume passes the negated unconsumed delta saved by Time_Pause. */
void Timer::OffsetBase(double ticks)
{
    baseTsc += (s64)ticks;
}

/* samples (so it also advances deltaTsc), returns the elapsed time. */
double Timer::GetElapsed(int unit)
{
    struct {
        s64 previous;
        double result;
    } work;
    if (running == 1) {
        work.previous = elapsedTsc;
        elapsedTsc = ReadTSC() - baseTsc;
        deltaTsc = elapsedTsc - work.previous;
        work.result = Convert(elapsedTsc, unit);
    } else {
        work.result = Convert(elapsedTsc, unit);
    }
    return work.result;
}

/* samples, returns the time since the previous sample; 0.0 while stopped. */
double Timer::GetDelta(int unit)
{
    struct {
        s64 previous;
        double result;
    } work;
    if (running == 1) {
        work.previous = elapsedTsc;
        elapsedTsc = ReadTSC() - baseTsc;
        deltaTsc = elapsedTsc - work.previous;
        work.result = Convert(deltaTsc, unit);
    } else {
        work.result = 0.0;
    }
    return work.result;
}

double Timer::PeekElapsed(int unit)
{
    return Convert(elapsedTsc, unit);
}

double Timer::PeekDelta(int unit)
{
    double result;
    if (running == 1)
        result = Convert(deltaTsc, unit);
    else
        result = 0.0;
    return result;
}

/* The performance counter (Windows', or SDL's on the other systems: src/platform/win32_sdl_compat.cpp) in place of
 * the original's RDTSC: steady under frequency scaling and on every CPU. g_cpuHz is its frequency (the constructor). */
s64 Timer::ReadTSC()
{
    s64 ticks = 0;
    QueryPerformanceCounter(&ticks);
    return ticks;
}

double Timer::Convert(s64 ticks, int unit)
{
    double result;
    switch (unit) {
        case TIMER_TICKS:
            result = (double)ticks;
            break;
        case TIMER_SECONDS:
            result = (double)ticks / g_cpuHz;
            break;
        case TIMER_MILLISECONDS:
            result = (double)(ticks * 1000) / g_cpuHz;
            break;
        case TIMER_MICROSECONDS:
            result = (double)(ticks * 1000000) / g_cpuHz;
            break;
        default:
            result = -1.0;
            break;
    }
    return result;
}
