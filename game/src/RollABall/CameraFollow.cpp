#include <RollABall/CameraFollow.hpp>

#include <Canis/App.hpp>
#include <Canis/Scene.hpp>
#include <Canis/ConfigHelper.hpp>

namespace RollABall
{
    namespace
    {
        ScriptConf conf = {};
    }

    void RegisterCameraFollowScript(Canis::App& _app)
    {
        REGISTER_PROPERTY(conf, RollABall::CameraFollow, offset, Canis::Vector3);
        REGISTER_PROPERTY(conf, RollABall::CameraFollow, followSmoothing, float);

        DEFAULT_CONFIG_AND_REQUIRED(conf, RollABall::CameraFollow, Canis::Transform3D);

        conf.DrawInspector = [](Canis::Editor&, Canis::Entity& _entity, const ScriptConf& _conf) -> void
        {
            if (_entity.HasScript<RollABall::CameraFollow>())
            {
                RollABall::CameraFollow& follow = _entity.GetScript<RollABall::CameraFollow>();
                ImGui::InputFloat3(("offset##" + _conf.name).c_str(), &follow.offset.x, "%.3f");
                ImGui::InputFloat(("followSmoothing##" + _conf.name).c_str(), &follow.followSmoothing);
            }
        };

        _app.RegisterScript(conf);
    }

    DEFAULT_UNREGISTER_SCRIPT(conf, CameraFollow)

    void CameraFollow::Create() {}

    void CameraFollow::Ready()
    {
        m_transform = entity.HasComponent<Canis::Transform3D>() ? &entity.GetComponent<Canis::Transform3D>() : nullptr;
        TryFindPlayer();
    }

    void CameraFollow::Destroy() {}

    void CameraFollow::Update(float _dt)
    {
        if (m_transform == nullptr)
            m_transform = entity.HasComponent<Canis::Transform3D>() ? &entity.GetComponent<Canis::Transform3D>() : nullptr;

        if (m_transform == nullptr)
            return;

        if (m_playerTransform == nullptr || m_playerTransform->entity == nullptr || !m_playerTransform->entity->active)
            TryFindPlayer();

        if (m_playerTransform == nullptr)
            return;

        // Keep a fixed camera offset from the player (classic Roll-a-Ball behavior).
        const Canis::Vector3 targetPosition = m_playerTransform->GetGlobalPosition() + offset;
        const float blend = glm::clamp(followSmoothing * _dt, 0.0f, 1.0f);
        m_transform->position = glm::mix(m_transform->position, targetPosition, blend);
    }

    void CameraFollow::EditorInspectorDraw() {}

    void CameraFollow::TryFindPlayer()
    {
        m_playerTransform = nullptr;

        if (entity.scene == nullptr)
            return;

        Canis::Entity* playerEntity = entity.scene->GetEntityWithTag("Player");
        if (playerEntity == nullptr || !playerEntity->HasComponent<Canis::Transform3D>())
            return;

        m_playerTransform = &playerEntity->GetComponent<Canis::Transform3D>();
    }
}
