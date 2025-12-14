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
        REGISTER_PROPERTY(conf, TankGame::Tank, speed, float);
        REGISTER_PROPERTY(conf, TankGame::Tank, turnSpeed, float);
        REGISTER_PROPERTY(conf, TankGame::Tank, count, int);

        conf.DEFAULT_NAME(TankGame::Tank);
        conf.DEFAULT_ADD_AND_REQUIRED(TankGame::Tank, Canis::RectTransform, Canis::Sprite2D);
        conf.DEFAULT_HAS(TankGame::Tank);
        conf.DEFAULT_REMOVE(TankGame::Tank);
        conf.DEFAULT_ENCODE(conf, TankGame::Tank);
        conf.DEFAULT_DECODE(conf, TankGame::Tank);
        conf.DrawInspector = [](Editor &_editor, Entity &_entity, const ScriptConf &_conf) -> void
        {
            TankGame::Tank *tank = nullptr;
            if ((tank = _entity.GetScript<TankGame::Tank>()) != nullptr)
            {
                ImGui::InputFloat(("speed##" + _conf.name).c_str(), &tank->speed);
                ImGui::InputFloat(("turnSpeed##" + _conf.name).c_str(), &tank->turnSpeed);
                ImGui::InputInt(("count##" + _conf.name).c_str(), &tank->count);
            }
        };

        _app.RegisterScript(conf);
    }

    DEFAULT_UNREGISTER_SCRIPT(conf, Tank)

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