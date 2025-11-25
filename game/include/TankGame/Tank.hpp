#pragma once

#include <Canis/Entity.hpp>

namespace TankGame
{
    class Tank : Canis::ScriptableEntity
    {
    public:
        float speed = 10.0f;

        Tank(Canis::Entity &_entity) : Canis::ScriptableEntity(_entity) {}

        void Create();
        void Ready();
        void Destroy();
        void Update(float _dt);
        void EditorInspectorDraw();
    };

    extern void RegisterTankScript(Canis::App& _app);
    extern void UnRegisterTankScript(Canis::App& _app);
}