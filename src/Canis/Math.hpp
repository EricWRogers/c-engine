#pragma once
#include <string>
#include <glm/glm.hpp>

namespace Canis
{
    namespace Math
    {
        extern std::string Vec3ToString(const glm::vec3* _value);
        extern float RandomFloat(float min, float max);
    }
}