#include <Canis/Math.hpp>
#include <functional>
#include <math.h>
#include <cmath>
#include <string>

namespace Canis
{
    // ======================= Vector2 =======================
    Vector2::Vector2(Vector3 _v)
    {
        x = _v.x;
        y = _v.y;
    }
    Vector2::Vector2(Vector4 _v)
    {
        x = _v.x;
        y = _v.y;
    }
    size_t Vector2::Hash() const
    {
        std::hash<float> h;
        return h(x) ^ (h(y) << 1);
    }

    float Vector2::Distance(const Vector2 &_other) const
    {
        return sqrtf((_other.x - x) * (_other.x - x) +
                     (_other.y - y) * (_other.y - y));
    }

    float Vector2::Magnitude() const
    {
        return std::sqrt(x * x + y * y);
    }

    Vector2 Vector2::Normalize()
    {
        float magnitude = Magnitude();
        *this = (magnitude != 0.0f) ? *this / magnitude : Vector2(0.0f);
        return *this;
    }

    Vector2 Vector2::Normalize(const Vector2 &_vector)
    {
        float magnitude = _vector.Magnitude();
        return (magnitude != 0.0f) ? _vector / magnitude : Vector2(0.0f);
    }

    const char *Vector2::ToCString() const
    {
        static thread_local std::string s;
        s = "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
        return s.c_str();
    }

    // arithmetic
    Vector2 Vector2::operator+(const Vector2 &rhs) const { return {x + rhs.x, y + rhs.y}; }
    Vector2 Vector2::operator-(const Vector2 &rhs) const { return {x - rhs.x, y - rhs.y}; }
    Vector2 Vector2::operator*(float scalar) const { return {x * scalar, y * scalar}; }
    Vector2 Vector2::operator/(float scalar) const { return {x / scalar, y / scalar}; }

    Vector2 &Vector2::operator+=(const Vector2 &rhs)
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }
    Vector2 &Vector2::operator-=(const Vector2 &rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }
    Vector2 &Vector2::operator*=(float scalar)
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }
    Vector2 &Vector2::operator/=(float scalar)
    {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    // ======================= Vector3 =======================
    size_t Vector3::Hash() const
    {
        std::hash<float> h;
        return h(x) ^ (h(y) << 1) ^ (h(z) << 2);
    }

    float Vector3::Distance(const Vector3 &_other) const
    {
        return sqrtf((_other.x - x) * (_other.x - x) +
                     (_other.y - y) * (_other.y - y) +
                     (_other.z - z) * (_other.z - z));
    }

    const char *Vector3::ToCString() const
    {
        static thread_local std::string s;
        s = "(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")";
        return s.c_str();
    }

    // arithmetic
    Vector3 Vector3::operator+(const Vector3 &rhs) const { return {x + rhs.x, y + rhs.y, z + rhs.z}; }
    Vector3 Vector3::operator-(const Vector3 &rhs) const { return {x - rhs.x, y - rhs.y, z - rhs.z}; }
    Vector3 Vector3::operator*(float scalar) const { return {x * scalar, y * scalar, z * scalar}; }
    Vector3 Vector3::operator/(float scalar) const { return {x / scalar, y / scalar, z / scalar}; }

    Vector3 &Vector3::operator+=(const Vector3 &rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }
    Vector3 &Vector3::operator-=(const Vector3 &rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }
    Vector3 &Vector3::operator*=(float scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }
    Vector3 &Vector3::operator/=(float scalar)
    {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        return *this;
    }

    // ======================= Vector4 =======================
    size_t Vector4::Hash() const
    {
        std::hash<float> h;
        return h(x) ^ (h(y) << 1) ^ (h(z) << 2) ^ (h(w) << 3);
    }

    const char *Vector4::ToCString() const
    {
        static thread_local std::string s;
        s = "(" + std::to_string(x) + ", " + std::to_string(y) + ", " +
            std::to_string(z) + ", " + std::to_string(w) + ")";
        return s.c_str();
    }

    // arithmetic
    Vector4 Vector4::operator+(const Vector4 &rhs) const { return {x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w}; }
    Vector4 Vector4::operator-(const Vector4 &rhs) const { return {x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w}; }
    Vector4 Vector4::operator*(float scalar) const { return {x * scalar, y * scalar, z * scalar, w * scalar}; }
    Vector4 Vector4::operator/(float scalar) const { return {x / scalar, y / scalar, z / scalar, w / scalar}; }

    Vector4 &Vector4::operator+=(const Vector4 &rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        w += rhs.w;
        return *this;
    }
    Vector4 &Vector4::operator-=(const Vector4 &rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        w -= rhs.w;
        return *this;
    }
    Vector4 &Vector4::operator*=(float scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        w *= scalar;
        return *this;
    }
    Vector4 &Vector4::operator/=(float scalar)
    {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        w /= scalar;
        return *this;
    }

    size_t Matrix4::Hash() const
    {
        std::hash<float> h;
        size_t acc = 0;
        for (int i = 0; i < 16; ++i)
        {
            acc ^= (h(m[i]) + 0x9e3779b97f4a7c15ULL + (acc << 6) + (acc >> 2));
        }
        return acc;
    }

    const char *Matrix4::ToCString() const
    {
        static thread_local std::string s;
        s.clear();
        s.reserve(256);
        // Pretty-print as 4 rows; remember storage is column-major
        for (int row = 0; row < 4; ++row)
        {
            s += "[ ";
            s += std::to_string(m[0 * 4 + row]);
            s += " ";
            s += std::to_string(m[1 * 4 + row]);
            s += " ";
            s += std::to_string(m[2 * 4 + row]);
            s += " ";
            s += std::to_string(m[3 * 4 + row]);
            s += " ]";
            if (row < 3)
                s += "\n";
        }
        return s.c_str();
    }

    // ----------------------- Operators -----------------------

    Matrix4 Matrix4::operator+(const Matrix4 &_rhs) const
    {
        Matrix4 r{};
        for (int i = 0; i < 16; ++i)
            r.m[i] = m[i] + _rhs.m[i];
        return r;
    }

    Matrix4 Matrix4::operator-(const Matrix4 &_rhs) const
    {
        Matrix4 r{};
        for (int i = 0; i < 16; ++i)
            r.m[i] = m[i] - _rhs.m[i];
        return r;
    }

    // Column-major, column-vector convention:
    // C = A * B  ⇒ columns of C are A * columns of B.
    Matrix4 Matrix4::operator*(const Matrix4 &_rhs) const
    {
        Matrix4 r{};
        for (int col = 0; col < 4; ++col)
        {
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

    Vector4 Matrix4::operator*(const Vector4 &_v) const
    {
        Vector4 out{};
        // out = M * v  (column vectors)
        out.x = m[0 * 4 + 0] * _v.x + m[1 * 4 + 0] * _v.y + m[2 * 4 + 0] * _v.z + m[3 * 4 + 0] * _v.w;
        out.y = m[0 * 4 + 1] * _v.x + m[1 * 4 + 1] * _v.y + m[2 * 4 + 1] * _v.z + m[3 * 4 + 1] * _v.w;
        out.z = m[0 * 4 + 2] * _v.x + m[1 * 4 + 2] * _v.y + m[2 * 4 + 2] * _v.z + m[3 * 4 + 2] * _v.w;
        out.w = m[0 * 4 + 3] * _v.x + m[1 * 4 + 3] * _v.y + m[2 * 4 + 3] * _v.z + m[3 * 4 + 3] * _v.w;
        return out;
    }

    Matrix4 Matrix4::operator*(float _scalar) const
    {
        Matrix4 r{};
        for (int i = 0; i < 16; ++i)
            r.m[i] = m[i] * _scalar;
        return r;
    }

    Matrix4 &Matrix4::operator+=(const Matrix4 &_rhs)
    {
        for (int i = 0; i < 16; ++i)
            m[i] += _rhs.m[i];
        return *this;
    }

    Matrix4 &Matrix4::operator-=(const Matrix4 &_rhs)
    {
        for (int i = 0; i < 16; ++i)
            m[i] -= _rhs.m[i];
        return *this;
    }

    Matrix4 &Matrix4::operator*=(const Matrix4 &_rhs)
    {
        *this = (*this) * _rhs;
        return *this;
    }

    Matrix4 &Matrix4::operator*=(float _scalar)
    {
        for (int i = 0; i < 16; ++i)
            m[i] *= _scalar;
        return *this;
    }

    void Matrix4::Identity()
    {
        m[0] = 1.0f;
        m[4] = 0.0f;
        m[8] = 0.0f;
        m[12] = 0.0f;
        m[1] = 0.0f;
        m[5] = 1.0f;
        m[9] = 0.0f;
        m[13] = 0.0f;
        m[2] = 0.0f;
        m[6] = 0.0f;
        m[10] = 1.0f;
        m[14] = 0.0f;
        m[3] = 0.0f;
        m[7] = 0.0f;
        m[11] = 0.0f;
        m[15] = 1.0f;
    }

    // Column-major translation lives in the last column
    void Matrix4::Translate(const Vector3 &_translation)
    {
        m[12] = _translation.x; // (0,3)
        m[13] = _translation.y; // (1,3)
        m[14] = _translation.z; // (2,3)
    }

    // Scale is diagonal in column-major
    void Matrix4::Scale(const Vector3 &_scale)
    {
        Matrix4 temp;
        temp.Identity();
        temp[0] = _scale.x;  // (0,0)
        temp[5] = _scale.y;  // (1,1)
        temp[10] = _scale.z; // (2,2)
        temp[15] = 1.0f;     // (3,3)
        *this = *this * temp;
    }

    static Matrix4 MakeRotation(float rad, const Vector3& axis) {
        Matrix4 M; M.Identity();
        // (standard Rodrigues formula, column-major)
        float len2 = axis.x*axis.x + axis.y*axis.y + axis.z*axis.z;
        if (len2 < 1e-20f) return M;
        float invLen = 1.0f / std::sqrt(len2);
        float x = axis.x * invLen, y = axis.y * invLen, z = axis.z * invLen;
        float c = std::cos(rad), s = std::sin(rad), t = 1.0f - c;

        // 3x3 submatrix
        M.m[0] = t*x*x + c;     M.m[4] = t*x*y - s*z; M.m[8]  = t*x*z + s*y;
        M.m[1] = t*y*x + s*z;   M.m[5] = t*y*y + c;   M.m[9]  = t*y*z - s*x;
        M.m[2] = t*z*x - s*y;   M.m[6] = t*z*y + s*x; M.m[10] = t*z*z + c;
        return M;
    }

    void Matrix4::Rotate(float _radians, const Vector3 &_axis)
    {
         *this = (*this) * MakeRotation(_radians, _axis); 
         return;
        const float len2 = _axis.x * _axis.x + _axis.y * _axis.y + _axis.z * _axis.z;
        if (len2 <= 1e-20f)
        {
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
        const float r00 = t * x * x + c;
        const float r01 = t * x * y - s * z;
        const float r02 = t * x * z + s * y;

        const float r10 = t * y * x + s * z;
        const float r11 = t * y * y + c;
        const float r12 = t * y * z - s * x;

        const float r20 = t * z * x - s * y;
        const float r21 = t * z * y + s * x;
        const float r22 = t * z * z + c;

        // Column 0
        m[0] = r00;
        m[1] = r10;
        m[2] = r20;
        m[3] = 0.0f;
        // Column 1
        m[4] = r01;
        m[5] = r11;
        m[6] = r21;
        m[7] = 0.0f;
        // Column 2
        m[8] = r02;
        m[9] = r12;
        m[10] = r22;
        m[11] = 0.0f;
        // Column 3
        m[12] = 0.0f;
        m[13] = 0.0f;
        m[14] = 0.0f;
        m[15] = 1.0f;
    }

    void Matrix4::Orthographic(float _left, float _right,
                               float _bottom, float _top,
                               float _near, float _far)
    {
        const float rl = (_right - _left);
        const float tb = (_top - _bottom);
        const float fn = (_far - _near);

        for (int i = 0; i < 16; ++i)
            m[i] = 0.0f;

        m[0] = 2.0f / rl;   // (0,0)
        m[5] = 2.0f / tb;   // (1,1)
        m[10] = -2.0f / fn; // (2,2)
        m[15] = 1.0f;       // (3,3)

        // Translation (last column)
        m[12] = -(_right + _left) / rl; // (0,3)
        m[13] = -(_top + _bottom) / tb; // (1,3)
        m[14] = -(_far + _near) / fn;   // (2,3)
    }

    void Matrix4::Perspective(float _fovRadians, float _aspect, float _near, float _far)
    {
        for (int i = 0; i < 16; ++i)
            m[i] = 0.0f;

        if (_aspect == 0.0f || _near == _far)
        {
            Identity();
            return;
        }

        const float tanHalfFov = std::tan(_fovRadians * 0.5f);
        if (tanHalfFov == 0.0f)
        {
            Identity();
            return;
        }

        m[0] = 1.0f / (_aspect * tanHalfFov);
        m[5] = 1.0f / tanHalfFov;
        m[10] = -(_far + _near) / (_far - _near);
        m[11] = -1.0f;
        m[14] = -(2.0f * _far * _near) / (_far - _near);
    }

    Vector2 RotatePoint(Vector2 _vector, float _radian)
    {
        float c = std::cos(_radian);
        float s = std::sin(_radian);
        return Vector2(
            _vector.x * c - _vector.y * s,
            _vector.x * s + _vector.y * c
        );
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

    void Clamp(int &_value, int _min, int _max)
    {
        if (_value < _min)
        {
            _value = _min;
        }

        if (_value > _max)
        {
            _value = _max;
        }
    }

    void Clamp(float &_value, float _min, float _max)
    {
        if (_value < _min)
        {
            _value = _min;
        }

        if (_value > _max)
        {
            _value = _max;
        }
    }

    void Clamp(size_t &_value, size_t _min, size_t _max)
    {
        if (_value < _min)
            _value = _min;
        
        if (_value > _max)
            _value = _max;
    }

    float Dot(const Vector3 &_a, const Vector3 &_b)
    {
        return _a.x * _b.x + _a.y * _b.y + _a.z * _b.z;
    }

    Vector3 Cross(const Vector3 &_a, const Vector3 &_b)
    {
        return Vector3(
            _a.y * _b.z - _a.z * _b.y,
            _a.z * _b.x - _a.x * _b.z,
            _a.x * _b.y - _a.y * _b.x
        );
    }

    Vector3 Normalize(const Vector3 &_vector)
    {
        const float length = std::sqrt(_vector.x * _vector.x + _vector.y * _vector.y + _vector.z * _vector.z);
        if (length <= 1e-8f)
            return Vector3(0.0f);

        return _vector / length;
    }

    Vector4 NormalizeQuaternion(const Vector4 &_quat)
    {
        const float length = std::sqrt(
            _quat.x * _quat.x +
            _quat.y * _quat.y +
            _quat.z * _quat.z +
            _quat.w * _quat.w
        );

        if (length <= 1e-8f)
            return Vector4(0.0f, 0.0f, 0.0f, 1.0f);

        return _quat / length;
    }

    Vector4 SlerpQuaternion(const Vector4 &_a, const Vector4 &_b, float _t)
    {
        Vector4 qa = NormalizeQuaternion(_a);
        Vector4 qb = NormalizeQuaternion(_b);

        float dot = qa.x * qb.x + qa.y * qb.y + qa.z * qb.z + qa.w * qb.w;
        if (dot < 0.0f)
        {
            dot = -dot;
            qb = qb * -1.0f;
        }

        if (dot > 0.9995f)
        {
            Vector4 linear = qa + ((qb - qa) * _t);
            return NormalizeQuaternion(linear);
        }

        dot = std::fmax(-1.0f, std::fmin(1.0f, dot));

        const float theta = std::acos(dot);
        const float sinTheta = std::sin(theta);
        if (sinTheta <= 1e-6f)
            return qa;

        const float aScale = std::sin((1.0f - _t) * theta) / sinTheta;
        const float bScale = std::sin(_t * theta) / sinTheta;

        return (qa * aScale) + (qb * bScale);
    }

    static Matrix4 QuaternionToMatrix(const Vector4 &_quat)
    {
        const Vector4 q = NormalizeQuaternion(_quat);
        const float xx = q.x * q.x;
        const float yy = q.y * q.y;
        const float zz = q.z * q.z;
        const float xy = q.x * q.y;
        const float xz = q.x * q.z;
        const float yz = q.y * q.z;
        const float wx = q.w * q.x;
        const float wy = q.w * q.y;
        const float wz = q.w * q.z;

        Matrix4 matrix;
        matrix.Identity();

        matrix[0] = 1.0f - 2.0f * (yy + zz);
        matrix[1] = 2.0f * (xy + wz);
        matrix[2] = 2.0f * (xz - wy);
        matrix[3] = 0.0f;

        matrix[4] = 2.0f * (xy - wz);
        matrix[5] = 1.0f - 2.0f * (xx + zz);
        matrix[6] = 2.0f * (yz + wx);
        matrix[7] = 0.0f;

        matrix[8] = 2.0f * (xz + wy);
        matrix[9] = 2.0f * (yz - wx);
        matrix[10] = 1.0f - 2.0f * (xx + yy);
        matrix[11] = 0.0f;

        matrix[12] = 0.0f;
        matrix[13] = 0.0f;
        matrix[14] = 0.0f;
        matrix[15] = 1.0f;

        return matrix;
    }

    Matrix4 TRS(const Vector3 &_translation, const Vector4 &_rotation, const Vector3 &_scale)
    {
        Matrix4 translationMatrix;
        translationMatrix.Identity();
        translationMatrix.Translate(_translation);

        Matrix4 rotationMatrix = QuaternionToMatrix(_rotation);

        Matrix4 scaleMatrix;
        scaleMatrix.Identity();
        scaleMatrix.Scale(_scale);

        return translationMatrix * rotationMatrix * scaleMatrix;
    }

    Matrix4 LookAt(const Vector3 &_eye, const Vector3 &_target, const Vector3 &_up)
    {
        const Vector3 f = Normalize(_target - _eye);
        const Vector3 s = Normalize(Cross(f, _up));
        const Vector3 u = Cross(s, f);

        Matrix4 matrix;
        matrix.Identity();

        matrix[0] = s.x;
        matrix[1] = u.x;
        matrix[2] = -f.x;
        matrix[3] = 0.0f;

        matrix[4] = s.y;
        matrix[5] = u.y;
        matrix[6] = -f.y;
        matrix[7] = 0.0f;

        matrix[8] = s.z;
        matrix[9] = u.z;
        matrix[10] = -f.z;
        matrix[11] = 0.0f;

        matrix[12] = -Dot(s, _eye);
        matrix[13] = -Dot(u, _eye);
        matrix[14] = Dot(f, _eye);
        matrix[15] = 1.0f;

        return matrix;
    }
}
