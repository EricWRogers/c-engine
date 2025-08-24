#include <Canis/Math.hpp>
#include <math.h>
#include <functional>

size_t Vector2D::Hash() const
{
    std::hash<float> hashFn;
    size_t hash1 = hashFn(x);
    size_t hash2 = hashFn(y);
    return hash1 ^ (hash2 << 1);
}

float Distance2D(const Vector2D &_v1, const Vector2D &_v2)
{
    return sqrt(powf(_v2.x - _v1.x, 2.0f) + powf(_v2.y - _v1.y, 2.0f));
}
