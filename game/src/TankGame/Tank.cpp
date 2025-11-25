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
    /*ScriptConf conf = {};

    void RegisterTankScript(Canis::App &_app)
    {
        Debug::Log("RTS");
        Debug::Log("RTS");
        REGISTER_PROPERTY(TankGame::Tank, speed, float);

        conf.DEFAULT_NAME(TankGame::Tank);
        conf.Add = [](Entity &_entity) -> void
        {
            // TODO: require a RectTransform component
            // TODO: require a Sprite2D component
            _entity.AddScript<TankGame::Tank>();
        };
        //conf.DEFAULT_ADD_AND_REQUIRED(TankGame::Tank, Canis::RectTransform, Canis::Sprite2D);
        conf.DEFAULT_HAS(TankGame::Tank);
        conf.DEFAULT_REMOVE(TankGame::Tank);
        conf.DEFAULT_GET(TankGame::Tank);
        conf.DEFAULT_ENCODE(TankGame::Tank);
        conf.DEFAULT_DECODE(TankGame::Tank);
        Debug::Log("RTS");
        conf.DrawInspector = [](Editor &_editor, Entity &_entity, const ScriptConf &_conf) -> void
        {
            Debug::Log("LOG");
            TankGame::Tank *tank = nullptr;
            if ((tank = _entity.GetScript<TankGame::Tank>()) != nullptr)
            {
                ImGui::InputFloat(("speed##" + _conf.name).c_str(), &tank->speed);
            }
        };
        Debug::Log("RTS");

        _app.RegisterScript(conf);
        Debug::Log("RTS");
    }

    DEFAULT_UNREGISTER_SCRIPT(Tank)*/

    ScriptConf conf = {
        .name = "TankGame::Tank",
        .Add = [](Entity &_entity) -> void
        {
            // TODO: require a RectTransform component
            // TODO: require a Sprite2D component
            _entity.AddScript<TankGame::Tank>();
        },
        .Has = [](Entity &_entity) -> bool
        { 
            return _entity.GetScript<TankGame::Tank>();
        },
        .Remove = [](Entity &_entity) -> void
        { _entity.RemoveScript<TankGame::Tank>(); },
        .Get = [](Entity& _entity) -> void* {
            return (void*)_entity.GetScript<TankGame::Tank>();
        },
        .Encode = [](YAML::Node &_node, Entity &_entity) -> void
        {
            Debug::Log("Check Encode");
            if (_entity.GetScript<TankGame::Tank>())
            {
                Debug::Log("Check Encode YES!!!!!!");
                TankGame::Tank &tank = *_entity.GetScript<TankGame::Tank>();

                YAML::Node comp;
                
                comp["speed"] = tank.speed;

                _node[conf.name] = comp;
            }
        },
        .Decode = [](YAML::Node &_node, Entity &_entity, bool _callCreate) -> void
        {
            if (auto paddleComponent = _node[conf.name])
            {
                auto &tank = *_entity.AddScript<TankGame::Tank>(false);
                tank.speed = paddleComponent["speed"].as<float>(tank.speed);
                if (_callCreate)
                    tank.Create();
                Debug::Log("speed: %f", tank.speed);
            }
        },
        .DrawInspector = [](Editor &_editor, Entity &_entity, const ScriptConf &_conf) -> void
        {
            TankGame::Tank *tank = nullptr;
            if ((tank = _entity.GetScript<TankGame::Tank>()) != nullptr)
            {
                ImGui::InputFloat(("speed##" + _conf.name).c_str(), &tank->speed);
            }
        },
    };

    void RegisterTankScript(Canis::App &_app)
    {
        _app.RegisterScript(conf);
    }

    void UnRegisterTankScript(Canis::App &_app)
    {
        _app.UnregisterScript(conf);
    }

    void Tank::Create() { Debug::Log("Tank But No Tank!"); }

    void Tank::Ready() {}

    void Tank::Destroy() { Debug::Log("Kill Tank But No Tank!"); }

    void Tank::Update(float _dt) { Debug::Log("Tank Update!"); }

    void Tank::EditorInspectorDraw() {}
}