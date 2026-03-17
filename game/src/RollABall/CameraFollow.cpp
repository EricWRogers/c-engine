#include <RollABall/CameraFollow.hpp>

#include <Canis/App.hpp>
#include <Canis/Scene.hpp>
#include <Canis/ConfigHelper.hpp>

namespace RollABall
{
    ScriptConf conf = {};

    void RegisterCameraFollowScript(Canis::App& _app)
    {
        REGISTER_PROPERTY(conf, RollABall::CameraFollow, offset);
        REGISTER_PROPERTY(conf, RollABall::CameraFollow, followSmoothing);

        DEFAULT_CONFIG_AND_REQUIRED(conf, RollABall::CameraFollow, Canis::Transform3D);

        conf.DEFAULT_DRAW_INSPECTOR(RollABall::CameraFollow);

        _app.RegisterScript(conf);
    }

    DEFAULT_UNREGISTER_SCRIPT(conf, CameraFollow)

    void CameraFollow::Create() {}

    void CameraFollow::Ready()
    {
        m_transform = entity.GetComponent<Canis::Transform3D>();
        TryFindPlayer();

        if (m_transform == nullptr || m_playerTransform == nullptr)
            return;
        
        offset = m_transform->position - m_playerTransform->GetGlobalPosition();
    }

    void CameraFollow::Destroy() {}

    void CameraFollow::Update(float _dt)
    {
        if (m_transform == nullptr || m_playerTransform == nullptr)
            return;

        const Canis::Vector3 targetPosition = m_playerTransform->GetGlobalPosition() + offset;
        const float blend = glm::clamp(followSmoothing * _dt, 0.0f, 1.0f);
        m_transform->position = glm::mix(m_transform->position, targetPosition, blend);
    }

    void CameraFollow::EditorInspectorDraw() {}

    void CameraFollow::TryFindPlayer()
    {
        Canis::Entity* playerEntity = entity.scene->GetEntityWithTag("Player");

        if (playerEntity == nullptr)
            return;

        m_playerTransform = playerEntity->GetComponent<Canis::Transform3D>();
    }
}
