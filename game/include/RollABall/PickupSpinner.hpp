#pragma once

#include <Canis/Entity.hpp>

namespace RollABall
{
    class PickupSpinner : public Canis::ScriptableEntity
    {
    private:
        Canis::Transform* m_transform = nullptr;
    public:
        static constexpr const char* ScriptName = "RollABall::PickupSpinner";

        float spinSpeedDegrees = 120.0f;

        explicit PickupSpinner(Canis::Entity& _entity) : Canis::ScriptableEntity(_entity) {}

        void Create() override;
        void Ready() override;
        void Destroy() override;
        void Update(float _dt) override;
        void EditorInspectorDraw() override;
    };

    void RegisterPickupSpinnerScript(Canis::App& _app);
    void UnRegisterPickupSpinnerScript(Canis::App& _app);
}
