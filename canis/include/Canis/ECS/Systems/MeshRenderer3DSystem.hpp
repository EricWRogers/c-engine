#pragma once
#include <Canis/System.hpp>

namespace Canis
{
    class Shader;

    class MeshRenderer3DSystem : public System
    {
    public:
        MeshRenderer3DSystem() : System() { m_name = type_name<MeshRenderer3DSystem>(); }

        void Create() override;
        void Update() override;

    private:
        Shader *m_shader = nullptr;
    };
} // end of Canis namespace
