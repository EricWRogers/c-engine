#pragma once

#include <Canis/Entity.hpp>

namespace TankGame
{
    class Tank : public Canis::ScriptableEntity
    {
    private:
        Canis::RectTransform* m_transform = nullptr;
        Canis::RectTransform* m_turret = nullptr;
    public:
        float speed = 10.0f;
        float turnSpeed = 25.0f;
        float test = 5.0f;

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