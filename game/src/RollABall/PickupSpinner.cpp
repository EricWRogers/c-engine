#include <RollABall/PickupSpinner.hpp>

#include <Canis/App.hpp>
#include <Canis/ConfigHelper.hpp>
#include <RollABall/PlayerController.hpp>

namespace RollABall
{
    namespace
    {
        Canis::Entity* FindEnteringPlayer(const std::vector<Canis::Entity*>& _entities)
        {
            for (Canis::Entity* other : _entities)
            {
                if (other == nullptr || !other->active)
                    continue;

                if (other->HasScript<RollABall::PlayerController>())
                    return other;
            }

            return nullptr;
        }
    } // namespace

    ScriptConf pickUpConf = {};

    void RegisterPickupSpinnerScript(Canis::App& _app)
    {
        REGISTER_PROPERTY(pickUpConf, RollABall::PickupSpinner, spinSpeedDegrees);

        DEFAULT_CONFIG_AND_REQUIRED(pickUpConf, RollABall::PickupSpinner, Canis::Transform);

        pickUpConf.DEFAULT_DRAW_INSPECTOR(RollABall::PickupSpinner);

        _app.RegisterScript(pickUpConf);
    }

    DEFAULT_UNREGISTER_SCRIPT(pickUpConf, PickupSpinner)

    void PickupSpinner::Create()
    {
        entity.GetComponent<Canis::Transform>();

        Canis::Rigidbody& rigidbody = entity.GetComponent<Canis::Rigidbody>();
        rigidbody.motionType = Canis::RigidbodyMotionType::STATIC;
        rigidbody.useGravity = false;
        rigidbody.isSensor = true;
        rigidbody.allowSleeping = false;
        rigidbody.linearVelocity = Vector3(0.0f);
        rigidbody.angularVelocity = Vector3(0.0f);

        if (!entity.HasComponent<Canis::BoxCollider>()
            && !entity.HasComponent<Canis::SphereCollider>()
            && !entity.HasComponent<Canis::CapsuleCollider>())
        {
            entity.GetComponent<Canis::BoxCollider>();
        }
    }

    void PickupSpinner::Ready() {}

    void PickupSpinner::Destroy() {}

    void PickupSpinner::Update(float _dt)
    {
        if (!entity.HasComponent<Canis::Transform>())
            return;

        Canis::Transform& transform = entity.GetComponent<Canis::Transform>();
        transform.rotation.y += spinSpeedDegrees * DEG2RAD * _dt;

        Canis::Entity* collectingPlayer = nullptr;
        if (entity.HasComponent<Canis::BoxCollider>())
            collectingPlayer = FindEnteringPlayer(entity.GetComponent<Canis::BoxCollider>().entered);

        if (collectingPlayer == nullptr)
            return;

        if (PlayerController* playerController = collectingPlayer->GetScript<PlayerController>())
        {
            playerController->CollectPickup();
            entity.Destroy();
        }
    }
}
