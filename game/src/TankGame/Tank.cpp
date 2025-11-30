#include <TankGame/Tank.hpp>

#include <Canis/App.hpp>
#include <Canis/Time.hpp>
#include <Canis/Scene.hpp>
#include <Canis/Window.hpp>
#include <Canis/Editor.hpp>
#include <Canis/InputManager.hpp>

#include <ConfigHelper.hpp>

using namespace Canis;

namespace TankGame
{
    ScriptConf conf = {};

    void RegisterTankScript(Canis::App &_app)
    {
        REGISTER_PROPERTY(TankGame::Tank, speed, float);

        conf.DEFAULT_NAME(TankGame::Tank);
        conf.DEFAULT_ADD_AND_REQUIRED(TankGame::Tank, Canis::RectTransform, Canis::Sprite2D);
        conf.DEFAULT_HAS(TankGame::Tank);
        conf.DEFAULT_REMOVE(TankGame::Tank);
        conf.DEFAULT_GET(TankGame::Tank);
        conf.DEFAULT_ENCODE(TankGame::Tank);
        conf.DEFAULT_DECODE(TankGame::Tank);
        
        conf.DrawInspector = [](Editor &_editor, Entity &_entity, const ScriptConf &_conf) -> void
        {
            Tank *tank = nullptr;
            if ((tank = _entity.GetScript<Tank>()) != nullptr)
            {
                ImGui::InputFloat(("speed##" + _conf.name).c_str(), &tank->speed);
            }
        };

        _app.RegisterScript(conf);

        //CHECK_SCRIPTABLE_ENTITY(TankGame::Tank);
    }

    DEFAULT_UNREGISTER_SCRIPT(Tank)

    void Tank::Create() { Debug::Log("Tank But No Tank!"); }

    void Tank::Ready() {}

    void Tank::Destroy() { Debug::Log("Kill Tank But No Tank!"); }

    void Tank::Update(float _dt) { Debug::Log("Tank Update!"); }

    void Tank::EditorInspectorDraw() {}
}