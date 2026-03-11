#pragma once

#include <Canis/Entity.hpp>

namespace Pong
{
    class Paddle : public Canis::ScriptableEntity
    {
    private:
        Canis::RectTransform* m_transform = nullptr;
        Canis::Sprite2D* m_sprite = nullptr;
    public:
        static constexpr const char* ScriptName = "Pong::Paddle";

        Canis::Vector2 direction;
        float speed = 100.0f;
        int playerNum = 1;
        Canis::Entity* ball = nullptr;

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
