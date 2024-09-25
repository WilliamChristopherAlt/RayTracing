#ifndef RANDOM_H
#define RANDOM_H

#include <cmath>
#include <chrono>
using namespace std::chrono;

#include <ctime>

uint32_t timeSinceEpochMillisec() {
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

float randomFloat()
{
    static uint32_t state = timeSinceEpochMillisec();
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    state ^= state << 3;
    return state / 4294967296.0f;
}

double randomFloat(float min, float max)
{
    return min + (max - min) * randomFloat();
}

int random_int(int min, int max)
{
    return floor(randomFloat(min, max));
}

#endif