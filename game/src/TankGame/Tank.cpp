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
        REGISTER_PROPERTY(TankGame::Tank, turnSpeed, float);

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
                ImGui::InputFloat(("turnSpeed##" + _conf.name).c_str(), &tank->turnSpeed);
            }
        };

        _app.RegisterScript(conf);

        //CHECK_SCRIPTABLE_ENTITY(TankGame::Tank);
    }

    DEFAULT_UNREGISTER_SCRIPT(Tank)

    void Tank::Create() { Debug::Log("Tank But No Tank!"); }

    void Tank::Ready() {
        m_transform = entity.GetScript<Canis::RectTransform>();
        m_turret = m_transform->children[0]->GetScript<RectTransform>();
    }

    void Tank::Destroy() { Debug::Log("Kill Tank But No Tank!"); }

    void Tank::Update(float _dt) {
        if (entity.scene->GetInputManager().GetKey(Canis::Key::W))
            m_transform->Move(Vector2(1.0f, 0.0f) * speed * _dt);
        
        if (entity.scene->GetInputManager().GetKey(Canis::Key::S))
            m_transform->Move(Vector2(-1.0f, 0.0f) * speed * _dt);
        
        if (entity.scene->GetInputManager().GetKey(Canis::Key::A))
            m_transform->rotation += turnSpeed * Canis::DEG2RAD * _dt;
        
        if (entity.scene->GetInputManager().GetKey(Canis::Key::D))
            m_transform->rotation += -turnSpeed * Canis::DEG2RAD * _dt;
    }

    void Tank::EditorInspectorDraw() {}
}