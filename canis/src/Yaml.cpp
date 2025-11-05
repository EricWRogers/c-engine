#include <Canis/Yaml.hpp>

namespace YAML
{
    Emitter &operator<<(Emitter &_out, const Canis::Vector2 &_vector)
    {
        _out << Flow;
        _out << BeginSeq << _vector.x << _vector.y << EndSeq;
        return _out;
    }

    Emitter &operator<<(Emitter &_out, const Canis::Vector3 &_vector)
    {
        _out << Flow;
        _out << BeginSeq << _vector.x << _vector.y << _vector.z << EndSeq;
        return _out;
    }

    Emitter &operator<<(Emitter &_out, const Canis::Vector4 &_vector)
    {
        _out << Flow;
        _out << BeginSeq << _vector.x << _vector.y << _vector.z << _vector.w << EndSeq;
        return _out;
    }
}