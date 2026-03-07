#pragma once
#include <Canis/System.hpp>
#include <Canis/ECS/RuntimeECS.hpp>

namespace Canis
{
    class SpriteAnimationSystem : public System
    {
    public:
        SpriteAnimationSystem() : System() { m_name = type_name<SpriteAnimationSystem>(); }

        void Create() override {}

        void Ready() override;

        void Update() override;

    private:
        RuntimeECSView m_animationView = {};
    };
} // end of Canis namespace
