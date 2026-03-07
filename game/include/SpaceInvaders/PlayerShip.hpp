#pragma once

#include <Canis/Entity.hpp>

namespace SpaceInvaders
{
    class PlayerShip : public Canis::ScriptableEntity
    {
    private:
        Canis::RectTransform* m_transform = nullptr;
        float m_fireTimer = 0.0f;

    public:
        static constexpr const char* ScriptName = "SpaceInvaders::PlayerShip";

        float speed = 300.0f;
        float fireCooldown = 0.2f;
        float bulletSpeed = 460.0f;
        float bulletLifetime = 2.0f;

        PlayerShip(Canis::Entity &_entity) : Canis::ScriptableEntity(_entity) {}

        void Create() override;
        void Ready() override;
        void Destroy() override;
        void Update(float _dt) override;
        void EditorInspectorDraw() override;

        void OnHit();
    };

    void RegisterPlayerShipScript(Canis::App& _app);
    void UnRegisterPlayerShipScript(Canis::App& _app);
}
