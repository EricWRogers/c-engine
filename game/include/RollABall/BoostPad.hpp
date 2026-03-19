#pragma once

#include <Canis/Entity.hpp>

namespace RollABall
{
    class BoostPad : public Canis::ScriptableEntity
    {
    public:
        static constexpr const char* ScriptName = "RollABall::BoostPad";

        float yeetForce = 10000.0f;

        explicit BoostPad(Canis::Entity& _entity) : Canis::ScriptableEntity(_entity) {}

        void Create() override;
        void Ready() override;
        void Destroy() override;
        void Update(float _dt) override;
        
        void CheckSensorEnter();
    };

    void RegisterBoostPadScript(Canis::App& _app);
    void UnRegisterBoostPadScript(Canis::App& _app);
}
