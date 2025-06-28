#include "Math.hpp"
#include "Debug.hpp"

#include <random>

namespace Canis
{
    namespace Math
    {
        std::string Vec3ToString(const glm::vec3* _value)
        {
            return glm::to_string(*_value);
        }

        float RandomFloat(float _min, float _max)
        {
            if (_max > _min)
            {
                float random = ((float)rand()) / (float)RAND_MAX;
                float range = _max - _min;
                return (random * range) + _min;
            }

            if (_min == _max)
                return _min;

            return RandomFloat(_max, _min);
        }
    }
}