#pragma once
#include <stdio.h>
#include <string>

// ---- Vector2 --------------------------------------------------------------
struct Vector2 {
    float x, y;

    // hashing / utils
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

// ---- Vector3  (dual view: xyz <-> rgb) ------------------------------------
struct Vector3 {
    union {
        struct { float x, y, z; };  // positional
        struct { float r, g, b; };  // color
        float v[3];                 // array access
    };

    // hashing / utils
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

// ---- Vector4  (dual view: xyzw <-> rgba) ----------------------------------
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

// ---- Matrix4 (row-major) ---------------------------------------------------
struct Matrix4 {
    float m[16]; // row-major: m[row*4 + col]

    // hashing / utils
    size_t Hash() const;
    const char* ToCString() const;

    // arithmetic
    Matrix4 operator+(const Matrix4& rhs) const;
    Matrix4 operator-(const Matrix4& rhs) const;
    Matrix4 operator*(const Matrix4& rhs) const; // matrix * matrix
    Vector4 operator*(const Vector4& v) const; // matrix * vec4
    Matrix4 operator*(float scalar) const;

    Matrix4& operator+=(const Matrix4& rhs);
    Matrix4& operator-=(const Matrix4& rhs);
    Matrix4& operator*=(const Matrix4& rhs);
    Matrix4& operator*=(float scalar);
};
