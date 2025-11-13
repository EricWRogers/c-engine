#pragma once

#include <Canis/Entity.hpp>

namespace Pong
{
    class Paddle : public Canis::ScriptableEntity
    {
    public:
        Canis::Vector2 direction;
        float speed = 100.0f;
        int playerNum = 1;

        Canis::RectTransform &transform = *entity.GetScript<Canis::RectTransform>();
        Canis::Sprite2D &sprite = *entity.GetScript<Canis::Sprite2D>();

        Paddle(Canis::Entity &_entity) : Canis::ScriptableEntity(_entity) {}

        void Create();
        void Ready();
        void Destroy();
        void Update(float _dt);
        void EditorInspectorDraw();
    };

    extern void RegisterPaddleScript(Canis::App& _app);
    extern void UnRegisterPaddleScript(Canis::App& _app);
} // namespace Pong