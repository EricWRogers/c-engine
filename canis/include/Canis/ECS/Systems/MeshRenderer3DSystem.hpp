#pragma once
#include <Canis/System.hpp>
#include <Canis/ECS/RuntimeECS.hpp>

namespace Canis
{
    class Shader;

    class MeshRenderer3DSystem : public System
    {
    public:
        MeshRenderer3DSystem() : System() { m_name = type_name<MeshRenderer3DSystem>(); }

        void Create() override;
        void Ready() override;
        void Update() override;

    private:
        Shader *m_shader = nullptr;
        RuntimeECSView m_cameraView = {};
        RuntimeECSView m_renderablesView = {};
        RuntimeECSView m_directionalLightsView = {};
        RuntimeECSView m_pointLightsView = {};
    };
} // end of Canis namespace
