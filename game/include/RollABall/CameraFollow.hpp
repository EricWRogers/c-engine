#pragma once

#include <Canis/Entity.hpp>

namespace RollABall
{
    class CameraFollow : public Canis::ScriptableEntity
    {
    private:
        Canis::Transform3D* m_transform = nullptr;
        Canis::Transform3D* m_playerTransform = nullptr;

        void TryFindPlayer();
    public:
        static constexpr const char* ScriptName = "RollABall::CameraFollow";

        Canis::Vector3 offset = Canis::Vector3(0.0f, 8.0f, 10.0f);
        float followSmoothing = 8.0f;

        explicit CameraFollow(Canis::Entity& _entity) : Canis::ScriptableEntity(_entity) {}

        void Create() override;
        void Ready() override;
        void Destroy() override;
        void Update(float _dt) override;
        void EditorInspectorDraw() override;
    };

    void RegisterCameraFollowScript(Canis::App& _app);
    void UnRegisterCameraFollowScript(Canis::App& _app);
}
