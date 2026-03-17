#include <RollABall/PlayerController.hpp>

#include <Canis/App.hpp>
#include <Canis/Scene.hpp>
#include <Canis/InputManager.hpp>
#include <Canis/Debug.hpp>
#include <Canis/ConfigHelper.hpp>

namespace RollABall
{
    namespace
    {
        ScriptConf conf = {};
    }

    void RegisterPlayerControllerScript(Canis::App& _app)
    {
        REGISTER_PROPERTY(conf, RollABall::PlayerController, moveForce);
        REGISTER_PROPERTY(conf, RollABall::PlayerController, pickupRadius);
        REGISTER_PROPERTY(conf, RollABall::PlayerController, logProgress);

        DEFAULT_CONFIG_AND_REQUIRED(conf, RollABall::PlayerController, Canis::Transform3D, Canis::Rigidbody3D);

        conf.DEFAULT_DRAW_INSPECTOR(RollABall::PlayerController,
            ImGui::Text("Collected: %d / %d", component->collectedPickups, component->totalPickups);
            ImGui::Text("State: %s", component->hasWon ? "You Win" : "Collecting");
        );

        _app.RegisterScript(conf);
    }

    DEFAULT_UNREGISTER_SCRIPT(conf, PlayerController)

    void PlayerController::Create() {}

    void PlayerController::Ready()
    {
        m_transform = entity.GetComponent<Canis::Transform3D>();
        m_rigidbody = entity.GetComponent<Canis::Rigidbody3D>();

        totalPickups = CountActivePickups();
        collectedPickups = 0;
        hasWon = (totalPickups == 0);

        if (logProgress)
        {
            if (totalPickups > 0)
                Canis::Debug::Log("Roll-a-Ball: Collect all %d pickups.", totalPickups);
            else
                Canis::Debug::Log("Roll-a-Ball: No pickups found in this scene.");
        }
    }

    void PlayerController::Destroy() {}

    void PlayerController::Update(float _dt)
    {
        m_transform = entity.GetComponent<Canis::Transform3D>();
        m_rigidbody = entity.GetComponent<Canis::Rigidbody3D>();

        if (m_transform == nullptr || m_rigidbody == nullptr || entity.scene == nullptr)
            return;

        Canis::InputManager& input = entity.scene->GetInputManager();

        float axisX = 0.0f;
        float axisZ = 0.0f;

        if (input.GetKey(Canis::Key::A) || input.GetKey(Canis::Key::LEFT))
            axisX -= 1.0f;
        if (input.GetKey(Canis::Key::D) || input.GetKey(Canis::Key::RIGHT))
            axisX += 1.0f;
        if (input.GetKey(Canis::Key::W) || input.GetKey(Canis::Key::UP))
            axisZ -= 1.0f;
        if (input.GetKey(Canis::Key::S) || input.GetKey(Canis::Key::DOWN))
            axisZ += 1.0f;

        Canis::Vector3 movement = Canis::Vector3(axisX, 0.0f, axisZ);
        if (glm::dot(movement, movement) > 0.0f)
        {
            // Unity-style movement: apply force from input every frame.
            movement = glm::normalize(movement);
            m_rigidbody->AddForce(movement * moveForce * _dt, Canis::Rigidbody3DForceMode::FORCE);
        }

        CollectNearbyPickups();
    }

    void PlayerController::EditorInspectorDraw() {}

    int PlayerController::CountActivePickups() const
    {
        return entity.scene->GetEntitiesWithTag("Pickup").size();
    }

    void PlayerController::CollectNearbyPickups()
    {
        if (entity.scene == nullptr || m_transform == nullptr || hasWon)
            return;

        const Canis::Vector3 playerPosition = m_transform->GetGlobalPosition();
        const float radiusSq = pickupRadius * pickupRadius;

        // Simple beginner-friendly pickup detection using a radius check.
        for (Canis::Entity* pickup : entity.scene->GetEntitiesWithTag("Pickup"))
        {
            if (pickup == nullptr || !pickup->active)
                continue;

            Canis::Transform3D* pickupTransform = pickup->GetComponent<Canis::Transform3D>();

            if (pickupTransform == nullptr)
                continue;

            const Canis::Vector3 delta = pickupTransform->GetGlobalPosition() - playerPosition;
            if (glm::dot(delta, delta) > radiusSq)
                continue;

            pickup->Destroy();
            ++collectedPickups;

            if (logProgress)
                Canis::Debug::Log("Roll-a-Ball: Pickups %d / %d", collectedPickups, totalPickups);

            if (collectedPickups >= totalPickups)
            {
                hasWon = true;
                Canis::Debug::Log("Roll-a-Ball: You Win!");
                break;
            }
        }
    }
}
