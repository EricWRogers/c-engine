#pragma once

#include <Canis/Entity.hpp>

namespace TankGame
{
    class FollowMouse : public Canis::ScriptableEntity
    {
    private:
        Canis::RectTransform* m_transform = nullptr;

    public:
        static constexpr const char* ScriptName = "TankGame::FollowMouse";

        Canis::Vector2 offset = Canis::Vector2(0.0f);

        FollowMouse(Canis::Entity &_entity) : Canis::ScriptableEntity(_entity) {}

        void Create();
        void Ready();
        void Destroy();
        void Update(float _dt);
    };

    extern void RegisterFollowMouseScript(Canis::App& _app);
    extern void UnRegisterFollowMouseScript(Canis::App& _app);
}
