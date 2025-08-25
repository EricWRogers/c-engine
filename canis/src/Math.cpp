#include <Canis/Math.hpp>
#include <functional>
#include <math.h>
#include <string>

// ======================= Vector2 =======================
size_t Vector2::Hash() const {
    std::hash<float> h;
    return h(x) ^ (h(y) << 1);
}

float Vector2::Distance2D(const Vector2& _other) const {
    return sqrtf((_other.x - x) * (_other.x - x) +
                 (_other.y - y) * (_other.y - y));
}

const char* Vector2::ToCString() const {
    static thread_local std::string s;
    s = "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
    return s.c_str();
}

// arithmetic
Vector2 Vector2::operator+(const Vector2& rhs) const { return {x + rhs.x, y + rhs.y}; }
Vector2 Vector2::operator-(const Vector2& rhs) const { return {x - rhs.x, y - rhs.y}; }
Vector2 Vector2::operator*(float scalar) const { return {x * scalar, y * scalar}; }
Vector2 Vector2::operator/(float scalar) const { return {x / scalar, y / scalar}; }

Vector2& Vector2::operator+=(const Vector2& rhs) { x += rhs.x; y += rhs.y; return *this; }
Vector2& Vector2::operator-=(const Vector2& rhs) { x -= rhs.x; y -= rhs.y; return *this; }
Vector2& Vector2::operator*=(float scalar) { x *= scalar; y *= scalar; return *this; }
Vector2& Vector2::operator/=(float scalar) { x /= scalar; y /= scalar; return *this; }

// ======================= Vector3 =======================
size_t Vector3::Hash() const {
    std::hash<float> h;
    return h(x) ^ (h(y) << 1) ^ (h(z) << 2);
}

const char* Vector3::ToCString() const {
    static thread_local std::string s;
    s = "(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")";
    return s.c_str();
}

// arithmetic
Vector3 Vector3::operator+(const Vector3& rhs) const { return {x + rhs.x, y + rhs.y, z + rhs.z}; }
Vector3 Vector3::operator-(const Vector3& rhs) const { return {x - rhs.x, y - rhs.y, z - rhs.z}; }
Vector3 Vector3::operator*(float scalar) const { return {x * scalar, y * scalar, z * scalar}; }
Vector3 Vector3::operator/(float scalar) const { return {x / scalar, y / scalar, z / scalar}; }

Vector3& Vector3::operator+=(const Vector3& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
Vector3& Vector3::operator-=(const Vector3& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
Vector3& Vector3::operator*=(float scalar) { x *= scalar; y *= scalar; z *= scalar; return *this; }
Vector3& Vector3::operator/=(float scalar) { x /= scalar; y /= scalar; z /= scalar; return *this; }

// ======================= Vector4 =======================
size_t Vector4::Hash() const {
    std::hash<float> h;
    return h(x) ^ (h(y) << 1) ^ (h(z) << 2) ^ (h(w) << 3);
}

const char* Vector4::ToCString() const {
    static thread_local std::string s;
    s = "(" + std::to_string(x) + ", " + std::to_string(y) + ", " +
        std::to_string(z) + ", " + std::to_string(w) + ")";
    return s.c_str();
}

// arithmetic
Vector4 Vector4::operator+(const Vector4& rhs) const { return {x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w}; }
Vector4 Vector4::operator-(const Vector4& rhs) const { return {x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w}; }
Vector4 Vector4::operator*(float scalar) const { return {x * scalar, y * scalar, z * scalar, w * scalar}; }
Vector4 Vector4::operator/(float scalar) const { return {x / scalar, y / scalar, z / scalar, w / scalar}; }

Vector4& Vector4::operator+=(const Vector4& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this; }
Vector4& Vector4::operator-=(const Vector4& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; return *this; }
Vector4& Vector4::operator*=(float scalar) { x *= scalar; y *= scalar; z *= scalar; w *= scalar; return *this; }
Vector4& Vector4::operator/=(float scalar) { x /= scalar; y /= scalar; z /= scalar; w /= scalar; return *this; }

// ======================= Matrix4 (row-major) =======================
size_t Matrix4::Hash() const {
    std::hash<float> h;
    size_t acc = 0;
    for (int i = 0; i < 16; ++i) {
        acc ^= (h(m[i]) << (i & 7));
    }
    return acc;
}

const char* Matrix4::ToCString() const {
    static thread_local std::string s;
    s.clear();
    s.reserve(256);
    for (int row = 0; row < 4; ++row) {
        s += "[ ";
        for (int col = 0; col < 4; ++col) {
            s += std::to_string(m[row * 4 + col]);
            s += (col < 3) ? " " : "";
        }
        s += " ]";
        if (row < 3) s += "\n";
    }
    return s.c_str();
}

Matrix4 Matrix4::operator+(const Matrix4& rhs) const {
    Matrix4 r{};
    for (int i = 0; i < 16; ++i) r.m[i] = m[i] + rhs.m[i];
    return r;
}

Matrix4 Matrix4::operator-(const Matrix4& rhs) const {
    Matrix4 r{};
    for (int i = 0; i < 16; ++i) r.m[i] = m[i] - rhs.m[i];
    return r;
}

Matrix4 Matrix4::operator*(const Matrix4& rhs) const {
    Matrix4 r{};
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            r.m[row * 4 + col] =
                m[row * 4 + 0] * rhs.m[0 * 4 + col] +
                m[row * 4 + 1] * rhs.m[1 * 4 + col] +
                m[row * 4 + 2] * rhs.m[2 * 4 + col] +
                m[row * 4 + 3] * rhs.m[3 * 4 + col];
        }
    }
    return r;
}

Vector4 Matrix4::operator*(const Vector4& v) const {
    return {
        m[0] * v.x + m[1] * v.y + m[2] * v.z + m[3] * v.w,
        m[4] * v.x + m[5] * v.y + m[6] * v.z + m[7] * v.w,
        m[8] * v.x + m[9] * v.y + m[10] * v.z + m[11] * v.w,
        m[12] * v.x + m[13] * v.y + m[14] * v.z + m[15] * v.w
    };
}

Matrix4 Matrix4::operator*(float scalar) const {
    Matrix4 r{};
    for (int i = 0; i < 16; ++i) r.m[i] = m[i] * scalar;
    return r;
}

Matrix4& Matrix4::operator+=(const Matrix4& rhs) { for (int i = 0; i < 16; ++i) m[i] += rhs.m[i]; return *this; }
Matrix4& Matrix4::operator-=(const Matrix4& rhs) { for (int i = 0; i < 16; ++i) m[i] -= rhs.m[i]; return *this; }
Matrix4& Matrix4::operator*=(const Matrix4& rhs) { *this = *this * rhs; return *this; }
Matrix4& Matrix4::operator*=(float scalar) { for (int i = 0; i < 16; ++i) m[i] *= scalar; return *this; }
