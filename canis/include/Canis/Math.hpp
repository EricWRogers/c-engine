
#pragma once
#include <stdio.h>
#include <string>
#include <math.h>
#include <functional>

#include <Canis/Data/Types.hpp>

namespace Canis
{
    const float PI = 3.14159265f;
    const float RAD2DEG = 180.0f / PI;
    const float DEG2RAD = PI / 180.0f;

    struct Vector3;
    struct Vector4;
    
    struct Vector2
    {
        float x, y;

        Vector2()
        {
            x = 0.0f;
            y = 0.0f;
        }
        Vector2(float _scalor)
        {
            x = _scalor;
            y = _scalor;
        }
        Vector2(float _x, float _y)
        {
            x = _x;
            y = _y;
        }
        Vector2(Vector3 _v);
        Vector2(Vector4 _v);

        size_t Hash() const;
        float Distance(const Vector2 &_other) const;
        float Magnitude() const;
        Vector2 Normalize();
        const char *ToCString() const;

        static Vector2 Normalize(const Vector2 &_vector);

        // arithmetic
        Vector2 operator+(const Vector2 &rhs) const;
        Vector2 operator-(const Vector2 &rhs) const;
        Vector2 operator*(float scalar) const;
        Vector2 operator/(float scalar) const;

        Vector2 operator-() const { return Vector2(-x, -y); }

        Vector2 &operator+=(const Vector2 &rhs);
        Vector2 &operator-=(const Vector2 &rhs);
        Vector2 &operator*=(float scalar);
        Vector2 &operator/=(float scalar);

        bool operator==(const Vector2 &_other) const { return x == _other.x && y == _other.y; }
        bool operator!=(const Vector2 &_other) const { return x != _other.x || y != _other.y; }
    };

    const Vector2 VECTOR2_ZERO;

    struct Vector3
    {
        union
        {
            struct
            {
                float x, y, z;
            }; // positional
            struct
            {
                float r, g, b;
            }; // color
            float v[3]; // array access
        };

        Vector3()
        {
            x = 0.0f;
            y = 0.0f;
            z = 0.0f;
        }
        Vector3(float _scalor)
        {
            x = _scalor;
            y = _scalor;
            z = _scalor;
        }
        Vector3(Vector2 _v, float _z)
        {
            x = _v.x;
            y = _v.y;
            z = _z;
        }
        Vector3(float _x, float _y, float _z)
        {
            x = _x;
            y = _y;
            z = _z;
        }

        size_t Hash() const;
        float Distance(const Vector3 &_other) const;
        const char *ToCString() const;

        // arithmetic
        Vector3 operator+(const Vector3 &rhs) const;
        Vector3 operator-(const Vector3 &rhs) const;
        Vector3 operator*(float scalar) const;
        Vector3 operator/(float scalar) const;

        Vector3 &operator+=(const Vector3 &rhs);
        Vector3 &operator-=(const Vector3 &rhs);
        Vector3 &operator*=(float scalar);
        Vector3 &operator/=(float scalar);

        bool operator==(const Vector3 &rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }
        bool operator!=(const Vector3 &rhs) const { return x != rhs.x || y != rhs.y || z != rhs.z; }
    };

    struct Vector4
    {
        union
        {
            struct
            {
                float x, y, z, w;
            }; // positional
            struct
            {
                float r, g, b, a;
            }; // color
            float v[4]; // array access
        };

        Vector4()
        {
            x = 0.0f;
            y = 0.0f;
            z = 0.0f;
            w = 0.0f;
        }
        Vector4(float _scalor)
        {
            x = _scalor;
            y = _scalor;
            z = _scalor;
            w = _scalor;
        }
        Vector4(float _x, float _y, float _z, float _w)
        {
            x = _x;
            y = _y;
            z = _z;
            w = _w;
        }

        size_t Hash() const;
        const char *ToCString() const;

        bool operator==(const Vector4 &rhs) const
        {
            return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
        }

        bool operator!=(const Vector4 &rhs) const
        {
            return !(*this == rhs);
        }

        Vector4 operator+(const Vector4 &rhs) const;
        Vector4 operator-(const Vector4 &rhs) const;
        Vector4 operator*(float scalar) const;
        Vector4 operator/(float scalar) const;

        Vector4 &operator+=(const Vector4 &rhs);
        Vector4 &operator-=(const Vector4 &rhs);
        Vector4 &operator*=(float scalar);
        Vector4 &operator/=(float scalar);
    };

    typedef Vector4 Color;

    struct Matrix4
    {
        float m[16]; // column major

        Matrix4() {};

        float &operator[](int _idx) { return m[_idx]; }
        const float &operator[](int _idx) const { return m[_idx]; }

        size_t Hash() const;
        const char *ToCString() const;

        bool operator==(const Matrix4 &rhs) const
        {
            for (int i = 0; i < 16; ++i)
            {
                if (m[i] != rhs.m[i])
                {
                    return false;
                }
            }
            return true;
        }

        bool operator!=(const Matrix4 &rhs) const
        {
            return !(*this == rhs);
        }

        Matrix4 operator+(const Matrix4 &_rhs) const;
        Matrix4 operator-(const Matrix4 &_rhs) const;
        Matrix4 operator*(const Matrix4 &_rhs) const; // matrix * matrix
        Vector4 operator*(const Vector4 &_v) const;   // matrix * vec4
        Matrix4 operator*(float _scalar) const;

        Matrix4 &operator+=(const Matrix4 &_rhs);
        Matrix4 &operator-=(const Matrix4 &_rhs);
        Matrix4 &operator*=(const Matrix4 &_rhs);
        Matrix4 &operator*=(float _scalar);

        void Identity();
        void Translate(const Vector3 &_translation);
        void Scale(const Vector3 &_scale);
        void Rotate(float _radians, const Vector3 &_axis);
        void Orthographic(float _left, float _right,
                          float _bottom, float _top,
                          float _near, float _far);
    };

    static Matrix4 IdentitiyMatrix4() {
        Matrix4 m;
        m.Identity();
        return m;
    }

    extern Vector2 RotatePoint(Vector2 _vector, float _radian);

    extern void RotatePoint(Vector2 &_point, const float &_cosAngle, const float &_sinAngle);

    extern void RotatePointAroundPivot(Vector2 &_point, const Vector2 &_pivot, float _radian);

    extern void Clamp(int &_value, int _min, int _max);

    extern void Clamp(float &_value, float _min, float _max);

    extern void Clamp(size_t &_value, size_t _min, size_t _max);
}

namespace std {
    template<>
    struct hash<Canis::Vector2> {
        size_t operator()(const Canis::Vector2 &v) const {
            return v.Hash();
        }
    };

    template<>
    struct hash<Canis::Vector3> {
        size_t operator()(const Canis::Vector3 &v) const {
            return v.Hash();
        }
    };

    template<>
    struct hash<Canis::Vector4> {
        size_t operator()(const Canis::Vector4 &v) const {
            return v.Hash();
        }
    };

    template<>
    struct hash<Canis::Matrix4> {
        size_t operator()(const Canis::Matrix4 &m) const {
            return m.Hash();
        }
    };
}
