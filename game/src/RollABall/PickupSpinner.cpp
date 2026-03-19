#include <RollABall/PickupSpinner.hpp>

#include <Canis/App.hpp>
#include <Canis/ConfigHelper.hpp>
#include <RollABall/PlayerController.hpp>

namespace RollABall
{
    ScriptConf pickUpConf = {};

    void RegisterPickupSpinnerScript(App& _app)
    {
        REGISTER_PROPERTY(pickUpConf, RollABall::PickupSpinner, spinSpeedDegrees);

        DEFAULT_CONFIG_AND_REQUIRED(pickUpConf, RollABall::PickupSpinner, Transform);

        pickUpConf.DEFAULT_DRAW_INSPECTOR(RollABall::PickupSpinner);

        _app.RegisterScript(pickUpConf);
    }

    DEFAULT_UNREGISTER_SCRIPT(pickUpConf, PickupSpinner)

    void PickupSpinner::Create()
    {
        entity.GetComponent<Transform>();

        Rigidbody& rigidbody = entity.GetComponent<Rigidbody>();
        rigidbody.motionType = RigidbodyMotionType::STATIC;
        rigidbody.useGravity = false;
        rigidbody.isSensor = true;
        rigidbody.allowSleeping = false;
        rigidbody.linearVelocity = Vector3(0.0f);
        rigidbody.angularVelocity = Vector3(0.0f);

        if (!entity.HasComponent<BoxCollider>()
            && !entity.HasComponent<SphereCollider>()
            && !entity.HasComponent<CapsuleCollider>())
        {
            entity.GetComponent<BoxCollider>();
        }
    }

    void PickupSpinner::Ready() {}

    void PickupSpinner::Destroy() {}

    void PickupSpinner::Update(float _dt)
    {
        if (!entity.HasComponent<Transform>())
            return;

        Transform& transform = entity.GetComponent<Transform>();
        transform.rotation.y += spinSpeedDegrees * DEG2RAD * _dt;

        CheckSensorEnter();
    }

    void PickupSpinner::CheckSensorEnter()
    {
        if (!entity.HasComponents<BoxCollider,Rigidbody>())
            return;

        Entity* collectingPlayer = nullptr;

        for (Entity* other : entity.GetComponent<BoxCollider>().entered)
        {
            if (!other->active)
                continue;

            if (other->HasScript<RollABall::PlayerController>()) {
                collectingPlayer = other;
                break;
            }
        }

        if (collectingPlayer == nullptr)
            return;

        if (PlayerController* playerController = collectingPlayer->GetScript<PlayerController>())
        {
            playerController->CollectPickup();
            entity.Destroy();
        }
    }
}
