#pragma once
#include <stdio.h>


struct Vector2D
{
    float x = 0.0f;
    float y = 0.0f;

    Vector2D() = default;
    Vector2D(float _s)
    {
        x = _s;
        y = _s;
    }
    Vector2D(float _x, float _y)
    {
        x = _x;
        y = _y;
    }
    Vector2D &operator*=(float s)
    {
        x *= s;
        y *= s;
        return (*this);
    }
    Vector2D &operator/=(float s)
    {
        x /= s;
        y /= s;
        return (*this);
    }
    Vector2D &operator+=(const Vector2D &v)
    {
        x += v.x;
        y += v.y;
        return (*this);
    }
    Vector2D &operator-=(const Vector2D &v)
    {
        x -= v.x;
        y -= v.y;
        return (*this);
    }
    
    size_t Hash() const;
};

inline Vector2D operator +(const Vector2D& _a, const Vector2D& _b) {
    return Vector2D(_a.x + _b.x, _a.y + _b.y);
}
inline Vector2D operator -(const Vector2D& _a, const Vector2D& _b) {
    return Vector2D(_a.x - _b.x, _a.y - _b.y);
}
inline Vector2D operator /(const Vector2D& _a, const float _b) {
    return Vector2D(_a.x / _b, _a.y / _b);
}
inline Vector2D operator *(const Vector2D& _a, const float _b) {
    return Vector2D(_a.x * _b, _a.y * _b);
}
inline bool operator ==(const Vector2D& _a, const Vector2D& _b) {
    return (_a.x == _b.x) && (_a.y == _b.y);
}

extern float Distance2D(const Vector2D &_v1, const Vector2D &_v2);
