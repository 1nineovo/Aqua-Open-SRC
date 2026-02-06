#pragma once
#include <__msvc_chrono.hpp>
#include <string>
#include <unordered_map>
#include <chrono>

namespace std::chrono {
using ticks = duration<int, ratio<1, 20>>;
}

using namespace std::chrono;

namespace TimerUtil {
inline unsigned __int64 getCurrentMs() {
    FILETIME f;
    GetSystemTimeAsFileTime(&f);
    (long long)f.dwHighDateTime;
    unsigned __int64 nano = ((__int64)f.dwHighDateTime << 32LL) + (__int64)f.dwLowDateTime;
    return (nano - 116444736000000000LL) / 10000;
}
template <typename T>
inline float getTime() {
    static steady_clock::time_point startTime = high_resolution_clock::now();
    steady_clock::time_point currentTime = high_resolution_clock::now();
    return (float)duration_cast<T>(currentTime - startTime).count();
}
template <typename T>
inline bool hasReached(float* lastScan, float timeToReach) {
    const float currentTime = getTime<T>();
    if(currentTime - *lastScan < timeToReach)
        return false;
    *lastScan = currentTime;
    return true;
}
}  // namespace TimerUtil