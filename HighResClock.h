#pragma once
#include <cstdint>
#include <chrono>

#if defined(_WIN32)
#include <windows.h>
#endif

// 统一高精度计时器
class HighResClock
{
public:
    static uint64_t now_ns()
    {
#if defined(__linux__)
        // struct timespec ts;
        // clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
        // return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
        return 0;

#elif defined(_WIN32)
        LARGE_INTEGER freq, counter;
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&counter);
        return (uint64_t)((double)counter.QuadPart * 1e9 / (double)freq.QuadPart);

#else
#error "Unsupported platform"
#endif
    }
};
