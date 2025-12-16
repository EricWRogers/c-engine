#pragma once

#include <Canis/Entity.hpp>

namespace TankGame
{
    class Bullet : public Canis::ScriptableEntity
    {
    private:
        Canis::RectTransform* m_transform = nullptr;
    public:
        float speed = 10.0f;
        float lifeTime = 25.0f;

        Bullet(Canis::Entity &_entity) : Canis::ScriptableEntity(_entity) {}

        void Create();
        void Ready();
        void Destroy();
        void Update(float _dt);
        void EditorInspectorDraw();
    };

    extern void RegisterBulletScript(Canis::App& _app);
    extern void UnRegisterBulletScript(Canis::App& _app);
}