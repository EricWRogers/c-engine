#pragma once

#include <Canis/Entity.hpp>

namespace SpaceInvaders
{
    class SwarmController : public Canis::ScriptableEntity
    {
    private:
        float m_fireTimer = 0.0f;
        int m_direction = 1;

    public:
        static constexpr const char* ScriptName = "SpaceInvaders::SwarmController";

        float horizontalSpeed = 50.0f;
        float stepDown = 26.0f;
        float fireInterval = 1.1f;
        float enemyBulletSpeed = 220.0f;
        float enemyBulletLifetime = 4.0f;

        SwarmController(Canis::Entity &_entity) : Canis::ScriptableEntity(_entity) {}

        void Create() override;
        void Ready() override;
        void Destroy() override;
        void Update(float _dt) override;
        void EditorInspectorDraw() override;
    };

    void RegisterSwarmControllerScript(Canis::App& _app);
    void UnRegisterSwarmControllerScript(Canis::App& _app);
}
