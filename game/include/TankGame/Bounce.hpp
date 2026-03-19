#pragma once

#include <Canis/Entity.hpp>

namespace TankGame
{
    class Bounce : public Canis::ScriptableEntity
    {
    private:
        Canis::RectTransform* m_transform = nullptr;
        Canis::Vector2 m_restScale = Canis::Vector2(1.0f);
        float m_elapsed = 0.0f;
        bool m_isPlaying = false;

    public:
        static constexpr const char* ScriptName = "TankGame::Bounce";

        float scaleMultiplier = 1.5f;
        float duration = 0.18f;

        Bounce(Canis::Entity &_entity) : Canis::ScriptableEntity(_entity) {}

        void Create();
        void Ready();
        void Destroy();
        void Update(float _dt);
    };

    extern void RegisterBounceScript(Canis::App& _app);
    extern void UnRegisterBounceScript(Canis::App& _app);
}
