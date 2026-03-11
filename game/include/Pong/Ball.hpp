#pragma once

#include <Canis/Entity.hpp>

namespace Pong
{
    class Ball : public Canis::ScriptableEntity
    {
    private:
        Canis::RectTransform* m_transform = nullptr;
        Canis::Sprite2D* m_sprite = nullptr;
    public:
        static constexpr const char* ScriptName = "Pong::Ball";

        Canis::Vector2 direction = Canis::Vector2(1.0f, 0.5f);
        float speed = 100.0f;
        float randomRotation = 0;
        float test = 150.0f;

        Ball(Canis::Entity &_entity) : Canis::ScriptableEntity(_entity) {}

        void Create();
        void Ready();
        void Destroy();
        void Update(float _dt);
        void EditorInspectorDraw();
        void CheckWalls();
    };

    extern void RegisterBallScript(Canis::App& _app);
    extern void UnRegisterBallScript(Canis::App& _app);
} // namespace Pong
