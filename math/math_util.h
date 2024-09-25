#ifndef MATH_UTIL_H
#define MATH_UTIL_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const float PI = atan(1.0) * 4.0f;

void print(const glm::vec3& v)
{
    std::cout << "vec3(" << v.x << ", " << v.y << ", " << v.z << ")" << std::endl;
}

float lerp(float a, float b, float t)
{
    return a + (b - a) * t;
} 

float bilinear(float x00, float x01, float x10, float x11, float u, float v)
{
    return lerp(lerp(x00, x01, u), lerp(x10, x11, u), v);
}

float trilinear(float x000, float x001, float u, float x010, float x011, float x100, float x101, float x110, float x111, float s, float t, float v)
{
    return lerp(lerp(lerp(x000, x001, s), lerp(x010, x011, s), t),
                lerp(lerp(x100, x101, s), lerp(x110, x111, s), t), v);
}

float fade(float t)
{
    return t * t * t * (t * (t*6.0f - 15.0f) + 10.0f);
}

float ease_out(float x)
{
    return std::fmax(0.0f, std::fmin(1.0f, 1.0f - pow(1.0f - x, 6.0f)));
}

float norm(float x)
{
    return (x + 1.0f) / 2.0f;
}

float clamp(float x, float min, float max)
{
    return std::max(std::min(x, max), min);
}

int clamp(int x, int min, int max)
{
    return std::max(std::min(x, max), min);
}

#endif