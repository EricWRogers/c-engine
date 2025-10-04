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

float Vector2::Magnitude() const {
    return std::sqrt(x * x + y * y);
}

Vector2 Vector2::Normalize() {
    float magnitude = Magnitude();
	*this = (magnitude != 0.0f) ? *this / magnitude : Vector2(0.0f);
    return *this;
}

Vector2 Vector2::Normalize(const Vector2 &_vector) {
    float magnitude = _vector.Magnitude();
	return (magnitude != 0.0f) ? _vector / magnitude : Vector2(0.0f);
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

size_t Matrix4::Hash() const {
    std::hash<float> h;
    size_t acc = 0;
    for (int i = 0; i < 16; ++i) {
        acc ^= (h(m[i]) + 0x9e3779b97f4a7c15ULL + (acc << 6) + (acc >> 2));
    }
    return acc;
}

const char* Matrix4::ToCString() const {
    static thread_local std::string s;
    s.clear();
    s.reserve(256);
    // Pretty-print as 4 rows; remember storage is column-major
    for (int row = 0; row < 4; ++row) {
        s += "[ ";
        s += std::to_string(m[0 * 4 + row]); s += " ";
        s += std::to_string(m[1 * 4 + row]); s += " ";
        s += std::to_string(m[2 * 4 + row]); s += " ";
        s += std::to_string(m[3 * 4 + row]);
        s += " ]";
        if (row < 3) s += "\n";
    }
    return s.c_str();
}

// ----------------------- Operators -----------------------

Matrix4 Matrix4::operator+(const Matrix4& _rhs) const {
    Matrix4 r{};
    for (int i = 0; i < 16; ++i) r.m[i] = m[i] + _rhs.m[i];
    return r;
}

Matrix4 Matrix4::operator-(const Matrix4& _rhs) const {
    Matrix4 r{};
    for (int i = 0; i < 16; ++i) r.m[i] = m[i] - _rhs.m[i];
    return r;
}

// Column-major, column-vector convention:
// C = A * B  ⇒ columns of C are A * columns of B.
Matrix4 Matrix4::operator*(const Matrix4& _rhs) const {
    Matrix4 r{};
    for (int col = 0; col < 4; ++col) {
        // Grab B's column 'col'
        const float b0 = _rhs.m[col * 4 + 0];
        const float b1 = _rhs.m[col * 4 + 1];
        const float b2 = _rhs.m[col * 4 + 2];
        const float b3 = _rhs.m[col * 4 + 3];
        // r(:,col) = A * [b0 b1 b2 b3]
        r.m[col * 4 + 0] = m[0 * 4 + 0] * b0 + m[1 * 4 + 0] * b1 + m[2 * 4 + 0] * b2 + m[3 * 4 + 0] * b3;
        r.m[col * 4 + 1] = m[0 * 4 + 1] * b0 + m[1 * 4 + 1] * b1 + m[2 * 4 + 1] * b2 + m[3 * 4 + 1] * b3;
        r.m[col * 4 + 2] = m[0 * 4 + 2] * b0 + m[1 * 4 + 2] * b1 + m[2 * 4 + 2] * b2 + m[3 * 4 + 2] * b3;
        r.m[col * 4 + 3] = m[0 * 4 + 3] * b0 + m[1 * 4 + 3] * b1 + m[2 * 4 + 3] * b2 + m[3 * 4 + 3] * b3;
    }
    return r;
}

Vector4 Matrix4::operator*(const Vector4& _v) const {
    Vector4 out{};
    // out = M * v  (column vectors)
    out.x = m[0 * 4 + 0] * _v.x + m[1 * 4 + 0] * _v.y + m[2 * 4 + 0] * _v.z + m[3 * 4 + 0] * _v.w;
    out.y = m[0 * 4 + 1] * _v.x + m[1 * 4 + 1] * _v.y + m[2 * 4 + 1] * _v.z + m[3 * 4 + 1] * _v.w;
    out.z = m[0 * 4 + 2] * _v.x + m[1 * 4 + 2] * _v.y + m[2 * 4 + 2] * _v.z + m[3 * 4 + 2] * _v.w;
    out.w = m[0 * 4 + 3] * _v.x + m[1 * 4 + 3] * _v.y + m[2 * 4 + 3] * _v.z + m[3 * 4 + 3] * _v.w;
    return out;
}

Matrix4 Matrix4::operator*(float _scalar) const {
    Matrix4 r{};
    for (int i = 0; i < 16; ++i) r.m[i] = m[i] * _scalar;
    return r;
}

Matrix4& Matrix4::operator+=(const Matrix4& _rhs) {
    for (int i = 0; i < 16; ++i) m[i] += _rhs.m[i];
    return *this;
}

Matrix4& Matrix4::operator-=(const Matrix4& _rhs) {
    for (int i = 0; i < 16; ++i) m[i] -= _rhs.m[i];
    return *this;
}

Matrix4& Matrix4::operator*=(const Matrix4& _rhs) {
    *this = (*this) * _rhs;
    return *this;
}

Matrix4& Matrix4::operator*=(float _scalar) {
    for (int i = 0; i < 16; ++i) m[i] *= _scalar;
    return *this;
}

void Matrix4::Identity() {
    m[0] = 1.0f; m[4] = 0.0f; m[8]  = 0.0f; m[12] = 0.0f;
    m[1] = 0.0f; m[5] = 1.0f; m[9]  = 0.0f; m[13] = 0.0f;
    m[2] = 0.0f; m[6] = 0.0f; m[10] = 1.0f; m[14] = 0.0f;
    m[3] = 0.0f; m[7] = 0.0f; m[11] = 0.0f; m[15] = 1.0f;
}

// Column-major translation lives in the last column
void Matrix4::Translate(const Vector3& _translation) {
    m[12] = _translation.x; // (0,3)
    m[13] = _translation.y; // (1,3)
    m[14] = _translation.z; // (2,3)
}

// Scale is diagonal in column-major
void Matrix4::Scale(const Vector3& _scale) {
    m[0]  = _scale.x;  // (0,0)
    m[5]  = _scale.y;  // (1,1)
    m[10] = _scale.z;  // (2,2)
    m[15] = 1.0f;      // (3,3)
}

void Matrix4::Rotate(float _radians, const Vector3& _axis) {
    const float len2 = _axis.x*_axis.x + _axis.y*_axis.y + _axis.z*_axis.z;
    if (len2 <= 1e-20f) {
        Identity();
        return;
    }
    const float invLen = 1.0f / std::sqrt(len2);
    const float x = _axis.x * invLen;
    const float y = _axis.y * invLen;
    const float z = _axis.z * invLen;

    const float c = std::cos(_radians);
    const float s = std::sin(_radians);
    const float t = 1.0f - c;

    // Build numeric rotation matrix R, then store by columns
    const float r00 = t*x*x + c;
    const float r01 = t*x*y - s*z;
    const float r02 = t*x*z + s*y;

    const float r10 = t*y*x + s*z;
    const float r11 = t*y*y + c;
    const float r12 = t*y*z - s*x;

    const float r20 = t*z*x - s*y;
    const float r21 = t*z*y + s*x;
    const float r22 = t*z*z + c;

    // Column 0
    m[0] = r00; m[1] = r10; m[2] = r20; m[3] = 0.0f;
    // Column 1
    m[4] = r01; m[5] = r11; m[6] = r21; m[7] = 0.0f;
    // Column 2
    m[8]  = r02; m[9]  = r12; m[10] = r22; m[11] = 0.0f;
    // Column 3
    m[12] = 0.0f; m[13] = 0.0f; m[14] = 0.0f; m[15] = 1.0f;
}

void Matrix4::Orthographic(float _left, float _right,
                           float _bottom, float _top,
                           float _near, float _far) {
    const float rl = (_right - _left);
    const float tb = (_top   - _bottom);
    const float fn = (_far   - _near);

    for (int i = 0; i < 16; ++i) m[i] = 0.0f;

    m[0]  =  2.0f / rl;   // (0,0)
    m[5]  =  2.0f / tb;   // (1,1)
    m[10] = -2.0f / fn;   // (2,2)
    m[15] =  1.0f;        // (3,3)

    // Translation (last column)
    m[12] = -(_right + _left) / rl; // (0,3)
    m[13] = -(_top   + _bottom) / tb; // (1,3)
    m[14] = -(_far   + _near) / fn; // (2,3)
}

void RotatePoint(Vector2 &_point, const float &_cosAngle, const float &_sinAngle)
{
    float x = _point.x;
    float y = _point.y;
    _point.x = x * _cosAngle - y * _sinAngle;
    _point.y = x * _sinAngle + y * _cosAngle;
}

void RotatePointAroundPivot(Vector2 &_point, const Vector2 &_pivot, float _radian)
{
    float s = sin(-_radian);
    float c = cos(-_radian);

    Vector2 holder = _point;

    holder -= _pivot;

    _point.x = holder.x * c - holder.y * s;
    _point.y = holder.x * s + holder.y * c;

    _point += _pivot;
}