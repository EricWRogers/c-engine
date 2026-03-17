#pragma once

#include <Canis/Entity.hpp>

namespace RollABall
{
    class PlayerController : public Canis::ScriptableEntity
    {
    private:
        Canis::Transform3D* m_transform = nullptr;
        Canis::Rigidbody3D* m_rigidbody = nullptr;

        int CountActivePickups() const;
    public:
        static constexpr const char* ScriptName = "RollABall::PlayerController";

        float moveForce = 35.0f;
        float pickupRadius = 1.15f;
        bool logProgress = true;

        int totalPickups = 0;
        int collectedPickups = 0;
        bool hasWon = false;
        bool sprint = false;

        explicit PlayerController(Canis::Entity& _entity) : Canis::ScriptableEntity(_entity) {}

        void Create() override;
        void Ready() override;
        void Destroy() override;
        void Update(float _dt) override;
        void EditorInspectorDraw() override;
    };

    void RegisterPlayerControllerScript(Canis::App& _app);
    void UnRegisterPlayerControllerScript(Canis::App& _app);
}
