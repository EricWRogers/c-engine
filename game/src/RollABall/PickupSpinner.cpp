#include <RollABall/PickupSpinner.hpp>

#include <Canis/App.hpp>
#include <Canis/ConfigHelper.hpp>

namespace RollABall
{
    namespace
    {
        ScriptConf conf = {};
    }

    void RegisterPickupSpinnerScript(Canis::App& _app)
    {
        REGISTER_PROPERTY(conf, RollABall::PickupSpinner, spinSpeedDegrees, float);

        DEFAULT_CONFIG_AND_REQUIRED(conf, RollABall::PickupSpinner, Canis::Transform3D);

        conf.DEFAULT_DRAW_INSPECTOR(RollABall::PickupSpinner);

        _app.RegisterScript(conf);
    }

    DEFAULT_UNREGISTER_SCRIPT(conf, PickupSpinner)

    void PickupSpinner::Create() {}

    void PickupSpinner::Ready()
    {
        m_transform = entity.HasComponent<Canis::Transform3D>() ? &entity.GetComponent<Canis::Transform3D>() : nullptr;
    }

    void PickupSpinner::Destroy() {}

    void PickupSpinner::Update(float _dt)
    {
        m_transform = entity.HasComponent<Canis::Transform3D>() ? &entity.GetComponent<Canis::Transform3D>() : nullptr;
        if (m_transform == nullptr)
            return;

        m_transform->rotation.y += spinSpeedDegrees * Canis::DEG2RAD * _dt;
    }

    void PickupSpinner::EditorInspectorDraw() {}
}
