#pragma once

#include <Canis/Entity.hpp>

namespace SpaceInvaders
{
    class Invader : public Canis::ScriptableEntity
    {
    private:
        Canis::RectTransform* m_transform = nullptr;
        float m_time = 0.0f;

    public:
        static constexpr const char* ScriptName = "SpaceInvaders::Invader";

        int points = 10;
        float wobbleAmplitude = 0.06f;
        float wobbleSpeed = 6.0f;

        Invader(Canis::Entity &_entity) : Canis::ScriptableEntity(_entity) {}

        void Create() override;
        void Ready() override;
        void Destroy() override;
        void Update(float _dt) override;
        void EditorInspectorDraw() override;
    };

    void RegisterInvaderScript(Canis::App& _app);
    void UnRegisterInvaderScript(Canis::App& _app);
}
