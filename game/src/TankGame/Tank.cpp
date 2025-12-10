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
    ScriptConf tankConf = {
        .name = "TankGame::Tank",
        .Add = [](Entity &_entity) -> void
        {
            // TODO: require a RectTransform component
            // TODO: require a Sprite2D component
            _entity.AddScript<TankGame::Tank>();
        },
        .Has = [](Entity &_entity) -> bool
        { return (_entity.GetScript<TankGame::Tank>() != nullptr); },
        .Remove = [](Entity &_entity) -> void
        { _entity.RemoveScript<TankGame::Tank>(); },
        .Encode = [](YAML::Node &_node, Entity &_entity) -> void
        {
            if (_entity.GetScript<TankGame::Tank>())
            {
                TankGame::Tank &tank = *_entity.GetScript<TankGame::Tank>();

                YAML::Node comp;

                comp["speed"] = tank.speed;
                comp["turnSpeed"] = tank.turnSpeed;
                comp["test"] = tank.test;

                _node[tankConf.name] = comp;
            }
        },
        .Decode = [](YAML::Node &_node, Entity &_entity, bool _callCreate) -> void
        {
            if (auto comp = _node[tankConf.name])
            {
                auto &tank = *_entity.AddScript<TankGame::Tank>(false);
                tank.speed = comp["speed"].as<float>(tank.speed);
                tank.turnSpeed = comp["turnSpeed"].as<float>(tank.turnSpeed);
                tank.test = comp["test"].as<float>(tank.test);

                if (_callCreate)
                    tank.Create();
            }
        },
        .DrawInspector = [](Editor &_editor, Entity &_entity, const ScriptConf &_conf) -> void
        {
            TankGame::Tank *tank = nullptr;
            if ((tank = _entity.GetScript<TankGame::Tank>()) != nullptr)
            {
                ImGui::InputFloat(("speed##" + _conf.name).c_str(), &tank->speed);
                ImGui::InputFloat(("turnSpeed##" + _conf.name).c_str(), &tank->turnSpeed);
                ImGui::InputFloat(("test##" + _conf.name).c_str(), &tank->test);
            }
        },
    };

    void RegisterTankScript(Canis::App &_app)
    {
        _app.RegisterScript(tankConf);
    }

    void UnRegisterTankScript(Canis::App &_app)
    {
        _app.UnregisterScript(tankConf);
    }
    /*ScriptConf conf = {};

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

    DEFAULT_UNREGISTER_SCRIPT(Tank)*/

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