#include <RollABall/PlayerController.hpp>

#include <Canis/App.hpp>
#include <Canis/Scene.hpp>
#include <Canis/InputManager.hpp>
#include <Canis/Debug.hpp>
#include <Canis/ConfigHelper.hpp>

namespace RollABall
{
    ScriptConf conf = {};

    void RegisterPlayerControllerScript(App& _app)
    {
        REGISTER_PROPERTY(conf, RollABall::PlayerController, moveForce);
        REGISTER_PROPERTY(conf, RollABall::PlayerController, pickupRadius);
        REGISTER_PROPERTY(conf, RollABall::PlayerController, logProgress);
        REGISTER_PROPERTY(conf, RollABall::PlayerController, sprint);

        DEFAULT_CONFIG_AND_REQUIRED(conf, RollABall::PlayerController, Transform3D, Rigidbody3D);

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
        totalPickups = CountActivePickups();
        collectedPickups = 0;
        hasWon = (totalPickups == 0);

        if (logProgress)
        {
            if (totalPickups > 0)
                Debug::Log("Roll-a-Ball: Collect all %d pickups.", totalPickups);
            else
                Debug::Log("Roll-a-Ball: No pickups found in this scene.");
        }
    }

    void PlayerController::Destroy() {}

    void PlayerController::Update(float _dt)
    {
        if (!entity.HasComponents<Transform3D, Rigidbody3D>())
            return;
        
        Transform3D& transform = entity.GetComponent<Transform3D>();
        Rigidbody3D& rigidbody = entity.GetComponent<Rigidbody3D>();

        InputManager& input = entity.scene->GetInputManager();

        Vector3 inputDirection = Vector3(0.0f);

        if (input.GetKey(Key::A) || input.GetKey(Key::LEFT))
            inputDirection.x -= 1.0f;
        if (input.GetKey(Key::D) || input.GetKey(Key::RIGHT))
            inputDirection.x += 1.0f;
        if (input.GetKey(Key::W) || input.GetKey(Key::UP))
            inputDirection.z -= 1.0f;
        if (input.GetKey(Key::S) || input.GetKey(Key::DOWN))
            inputDirection.z += 1.0f;
        
        sprint = input.GetKey(Key::LSHIFT);

        Vector3 movement = inputDirection;

        if (input.JustPressedKey(Key::SPACE))
            inputDirection.y = 500.0f;

        if (movement != Vector3(0.0f))
            movement = glm::normalize(movement);

        if (sprint)
            movement *= 2.0f;
        
        movement.y = inputDirection.y;

        rigidbody.AddForce(movement * moveForce * _dt, Rigidbody3DForceMode::FORCE);
        

        for (Entity* pickup : entity.scene->GetEntitiesWithTag("Pickup"))
        {
            if (!pickup->active || !pickup->HasComponent<Transform3D>())
                continue;
            
            Transform3D& pickupTransform = pickup->GetComponent<Transform3D>();

            float distance = glm::distance(pickupTransform.position, transform.position);

            if (distance < pickupRadius)
            {
                collectedPickups++;
                pickup->Destroy();

                hasWon = (collectedPickups >= totalPickups);
                break;
            }
        }
    }

    void PlayerController::EditorInspectorDraw() {}

    int PlayerController::CountActivePickups() const
    {
        return entity.scene->GetEntitiesWithTag("Pickup").size();
    }
}
