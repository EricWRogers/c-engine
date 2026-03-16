#include "../../include/Pong/Ball.hpp"
#include "../../include/Pong/Paddle.hpp"

#include <Canis/App.hpp>
#include <Canis/ConfigHelper.hpp>

using namespace Canis;

namespace Pong
{
    ScriptConf conf = {};

    void RegisterBallScript(Canis::App &_app)
    {
        REGISTER_PROPERTY(conf, Pong::Ball, direction, Vector2);
        REGISTER_PROPERTY(conf, Pong::Ball, speed, float);
        REGISTER_PROPERTY(conf, Pong::Ball, randomRotation, float);

        DEFAULT_CONFIG_AND_REQUIRED(conf, Pong::Ball, Canis::RectTransform, Canis::Sprite2D);

        conf.DrawInspector = [](Editor &_editor, Entity &_entity, const ScriptConf &_conf) -> void
        {
            Pong::Ball *ball = nullptr;
            ball = _entity.HasScript(Pong::Ball::ScriptName)
                ? &_entity.GetScript<Pong::Ball>()
                : nullptr;
            if (ball != nullptr)
            {
                ImGui::InputFloat2(("direction##" + _conf.name).c_str(), &ball->direction.x, "%.3f");
                ImGui::InputFloat(("speed##" + _conf.name).c_str(), &ball->speed);
                ImGui::InputFloat(("randomRotation##" + _conf.name).c_str(), &ball->randomRotation);
            }
        };

        _app.RegisterScript(conf);
    }

    DEFAULT_UNREGISTER_SCRIPT(conf, Ball)

ScriptConf paddleConf = {
    .name = "Pong::Paddle",
    .Construct = [](Entity& _entity, bool _callCreate) -> ScriptableEntity* {
        return _entity.AttachScript(Pong::Paddle::ScriptName, new Pong::Paddle(_entity), _callCreate);
    },
    .Add = [](Entity &_entity) -> void
    {
        // TODO: require a RectTransform component
        // TODO: require a Sprite2D component
        (void)_entity.AddScript<Paddle>();
    },
    .Has = [](Entity &_entity) -> bool
    { return _entity.HasScript(Paddle::ScriptName); },
    .Remove = [](Entity &_entity) -> void
    { _entity.RemoveScript<Paddle>(); },
    .Get = [](Entity& _entity) -> void* { return _entity.HasScript<Paddle>() ? (void*)&_entity.GetScript<Paddle>() : nullptr; },
    .Encode = [](YAML::Node &_node, Entity &_entity) -> void
    {
        Paddle* paddlePtr = _entity.HasScript<Paddle>() ? &_entity.GetScript<Paddle>() : nullptr;
        if (paddlePtr != nullptr)
        {
            Paddle &paddle = *paddlePtr;

            YAML::Node comp;

            comp["direction"] = paddle.direction;
            comp["speed"] = paddle.speed;
            comp["playerNum"] = paddle.playerNum;
            comp["ball"] = paddle.ball->uuid;

            _node[paddleConf.name] = comp;
        }
    },
    .Decode = [](YAML::Node &_node, Entity &_entity, bool _callCreate) -> void
    {
        if (auto paddleComponent = _node[paddleConf.name])
        {
            auto &paddle = _entity.AddScript<Paddle>(false);
            paddle.direction = paddleComponent["direction"].as<Vector2>(paddle.direction);
            paddle.speed = paddleComponent["speed"].as<float>(paddle.speed);
            paddle.playerNum = paddleComponent["playerNum"].as<int>(paddle.playerNum);
            _entity.scene->GetEntityAfterLoad(paddleComponent["ball"].as<Canis::UUID>(0),paddle.ball);
            if (_callCreate)
                paddle.Create();
        }
    },
    .DrawInspector = [](Editor &_editor, Entity &_entity, const ScriptConf &_conf) -> void
    {
        Paddle *paddle = _entity.HasScript<Paddle>() ? &_entity.GetScript<Paddle>() : nullptr;
        if (paddle != nullptr)
        {
            ImGui::InputFloat2(("direction##" + _conf.name).c_str(), &paddle->direction.x, "%.3f");
            ImGui::InputFloat(("speed##" + _conf.name).c_str(), &paddle->speed);
            ImGui::InputInt(("playerNum##" + _conf.name).c_str(), &paddle->playerNum, 0, 100);
            
            _editor.InputEntity("ball", paddle->ball);

        }
    },
};

void RegisterPaddleScript(Canis::App &_app)
{
    _app.RegisterScript(paddleConf);
}

void UnRegisterPaddleScript(Canis::App &_app)
{
    _app.UnregisterScript(paddleConf);
}
}
