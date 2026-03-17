#include <RollABall/PickupSpinner.hpp>

#include <Canis/App.hpp>
#include <Canis/ConfigHelper.hpp>

namespace RollABall
{
    ScriptConf pickUpConf = {};

    void RegisterPickupSpinnerScript(Canis::App& _app)
    {
        REGISTER_PROPERTY(pickUpConf, RollABall::PickupSpinner, spinSpeedDegrees);

        DEFAULT_CONFIG_AND_REQUIRED(pickUpConf, RollABall::PickupSpinner, Canis::Transform3D);

        pickUpConf.DEFAULT_DRAW_INSPECTOR(RollABall::PickupSpinner);

        _app.RegisterScript(pickUpConf);
    }

    DEFAULT_UNREGISTER_SCRIPT(pickUpConf, PickupSpinner)

    void PickupSpinner::Create() {}

    void PickupSpinner::Ready()
    {
        m_transform = entity.GetComponent<Canis::Transform3D>();
    }

    void PickupSpinner::Destroy() {}

    void PickupSpinner::Update(float _dt)
    {
        
    }

    void PickupSpinner::EditorInspectorDraw() {}
}
