#include <RollABall/BoostPad.hpp>

#include <Canis/App.hpp>
#include <Canis/ConfigHelper.hpp>
#include <RollABall/PlayerController.hpp>

namespace RollABall
{
    ScriptConf boostPadConf = {};

    void RegisterBoostPadScript(App& _app)
    {
        REGISTER_PROPERTY(boostPadConf, RollABall::BoostPad, yeetForce);
        REGISTER_PROPERTY(boostPadConf, RollABall::BoostPad, player);
        REGISTER_PROPERTY(boostPadConf, RollABall::BoostPad, ground);

        DEFAULT_CONFIG_AND_REQUIRED(boostPadConf, RollABall::BoostPad, Transform, BoxCollider);

        boostPadConf.DEFAULT_DRAW_INSPECTOR(RollABall::BoostPad);

        _app.RegisterScript(boostPadConf);
    }

    DEFAULT_UNREGISTER_SCRIPT(boostPadConf, BoostPad)

    void BoostPad::Create()
    {
        entity.GetComponent<Transform>();

        Rigidbody& rigidbody = entity.GetComponent<Rigidbody>();
        rigidbody.motionType = RigidbodyMotionType::STATIC;
        rigidbody.useGravity = false;
        rigidbody.isSensor = true;
        rigidbody.allowSleeping = false;
        rigidbody.linearVelocity = Vector3(0.0f);
        rigidbody.angularVelocity = Vector3(0.0f);
    }

    void BoostPad::Ready() {
        player = entity.scene.FindEntityWithName("Player");
    }

    void BoostPad::Destroy() {}

    void BoostPad::Update(float _dt)
    {
        CheckSensorEnter();
    }

    void BoostPad::CheckSensorEnter()
    {
        if (!entity.HasComponents<BoxCollider,Rigidbody>())
            return;

        Entity* collectingPlayer = nullptr;

        for (Entity* other : entity.GetComponent<BoxCollider>().entered)
        {
            if (!other->active)
                continue;

            if (other->HasScript<RollABall::PlayerController>()) {
                Rigidbody& playerRigidbody = other->GetComponent<Rigidbody>();
                playerRigidbody.AddForce(Vector3(0.0f, yeetForce*yeetForce, 0.0f), Rigidbody3DForceMode::IMPULSE);
                break;
            }
        }
    }
}
