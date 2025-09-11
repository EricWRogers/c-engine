
#pragma once
#include <stdio.h>
#include <string>

struct Vector2 {
    float x, y;

    Vector2() { x = 0.0f; y = 0.0f; }
    Vector2(float _scalor) { x = _scalor; y = _scalor; }
    Vector2(float _x, float _y) { x = _x; y = _y; }

    size_t Hash() const;
    float Distance2D(const Vector2& _other) const;
    const char* ToCString() const;

    // arithmetic
    Vector2 operator+(const Vector2& rhs) const;
    Vector2 operator-(const Vector2& rhs) const;
    Vector2 operator*(float scalar) const;
    Vector2 operator/(float scalar) const;

    Vector2& operator+=(const Vector2& rhs);
    Vector2& operator-=(const Vector2& rhs);
    Vector2& operator*=(float scalar);
    Vector2& operator/=(float scalar);
};

struct Vector3 {
    union {
        struct { float x, y, z; };  // positional
        struct { float r, g, b; };  // color
        float v[3];                 // array access
    };

    Vector3() { x = 0.0f; y = 0.0f; z = 0.0f; }
    Vector3(float _scalor) { x = _scalor; y = _scalor; z = _scalor; }
    Vector3(float _x, float _y, float _z) { x = _x; y = _y; z = _z; }

    size_t Hash() const;
    const char* ToCString() const;

    // arithmetic
    Vector3 operator+(const Vector3& rhs) const;
    Vector3 operator-(const Vector3& rhs) const;
    Vector3 operator*(float scalar) const;
    Vector3 operator/(float scalar) const;

    Vector3& operator+=(const Vector3& rhs);
    Vector3& operator-=(const Vector3& rhs);
    Vector3& operator*=(float scalar);
    Vector3& operator/=(float scalar);
};

struct Vector4 {
    union {
        struct { float x, y, z, w; }; // positional
        struct { float r, g, b, a; }; // color
        float v[4];                   // array access
    };

    // hashing / utils
    size_t Hash() const;
    const char* ToCString() const;

    // arithmetic
    Vector4 operator+(const Vector4& rhs) const;
    Vector4 operator-(const Vector4& rhs) const;
    Vector4 operator*(float scalar) const;
    Vector4 operator/(float scalar) const;

    Vector4& operator+=(const Vector4& rhs);
    Vector4& operator-=(const Vector4& rhs);
    Vector4& operator*=(float scalar);
    Vector4& operator/=(float scalar);
};

struct Matrix4 {
    float m[16]; // column major

    float& operator[](int _idx) { return m[_idx]; }
    const float& operator[](int _idx) const { return m[_idx]; }

    size_t Hash() const;
    const char* ToCString() const;

    Matrix4 operator+(const Matrix4& _rhs) const;
    Matrix4 operator-(const Matrix4& _rhs) const;
    Matrix4 operator*(const Matrix4& _rhs) const; // matrix * matrix
    Vector4 operator*(const Vector4& _v) const; // matrix * vec4
    Matrix4 operator*(float _scalar) const;

    Matrix4& operator+=(const Matrix4& _rhs);
    Matrix4& operator-=(const Matrix4& _rhs);
    Matrix4& operator*=(const Matrix4& _rhs);
    Matrix4& operator*=(float _scalar);

    void Identity();
    void Translate(const Vector3& _translation);
    void Scale(const Vector3& _scale);
    void Rotate(float _radians, const Vector3& _axis);
    void Orthographic(float _left, float _right,
                                float _bottom, float _top,
                                float _near, float _far);
};
