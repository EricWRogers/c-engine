#include "../../include/Pong/Ball.hpp"

#include <Canis/App.hpp>
#include <Canis/Time.hpp>
#include <Canis/Window.hpp>
#include <Canis/InputManager.hpp>

using namespace Canis;

namespace Pong
{

ScriptConf ballConf = {
    .name = "Pong::Ball",
    .Add = [](Entity &_entity) -> void
    {
        // TODO: require a RectTransform component
        // TODO: require a Sprite2D component
        _entity.AddScript<Ball>();
    },
    .Has = [](Entity &_entity) -> bool
    { return (_entity.GetScript<Ball>() != nullptr); },
    .Remove = [](Entity &_entity) -> void
    { _entity.RemoveScript<Ball>(); },
    .Encode = [](YAML::Node &_node, Entity &_entity) -> void
    {
        if (_entity.GetScript<Ball>())
        {
            Ball &ball = *_entity.GetScript<Ball>();

            YAML::Node comp;

            comp["direction"] = ball.direction;
            comp["speed"] = ball.speed;
            comp["randomRotation"] = ball.randomRotation;

            _node[ballConf.name] = comp;
        }
    },
    .Decode = [](YAML::Node &_node, Entity &_entity) -> void
    {
        if (auto ballComponent = _node[ballConf.name])
        {
            auto &ball = *_entity.AddScript<Ball>(false);
            ball.direction = ballComponent["direction"].as<Vector2>();
            ball.speed = ballComponent["speed"].as<float>();
            ball.randomRotation = ballComponent["randomRotation"].as<float>();
            ball.Create();
        }
    },
    .DrawInspector = [](Editor &_editor, Entity &_entity, const ScriptConf &_conf) -> void
    {
        Ball *ball = nullptr;
        if ((ball = _entity.GetScript<Ball>()) != nullptr)
        {
            ImGui::InputFloat2(("direction##" + _conf.name).c_str(), &ball->direction.x, "%.3f");
            ImGui::InputFloat(("speed##" + _conf.name).c_str(), &ball->speed);
            ImGui::InputFloat(("randomRotation##" + _conf.name).c_str(), &ball->randomRotation);
        }
    },
};

void RegisterBallScript(Canis::App &_app)
{
    _app.RegisterScript(ballConf);
}

void UnRegisterBallScript(Canis::App &_app)
{
    _app.UnregisterScript(ballConf);
}

void Ball::Create()
{
    direction = Vector2(-1.0f, -1.0f).Normalize();
    direction = Vector2::Normalize(direction);
}

void Ball::Ready()
{
    direction = Vector2(-1.0f, -1.0f).Normalize();
    direction = Vector2::Normalize(direction);
}

void Ball::Destroy() {}

void Ball::Update(float _dt)
{
    Vector2 delta = direction * speed * Time::DeltaTime();
    transform.position += delta;
    transform.rotation += DEG2RAD * 5000.0f * Time::DeltaTime();

    CheckWalls();
}

void Ball::EditorInspectorDraw()
{
    std::string nameOfType = "Ball";
    ImGui::Text("%s", nameOfType.c_str());
    ImGui::InputFloat2("direction", &direction.x, "%.3f");
    ImGui::InputFloat("speed", &speed);
    ImGui::InputFloat("randomRotation", &randomRotation);
}

void Ball::CheckWalls()
{
    Canis::Window &window = entity.scene->GetWindow();

    if (window.GetScreenWidth() * 0.5f <= transform.position.x + transform.size.x * 0.5f)
    {
        if (direction.x > 0.0f)
        {
            direction.x *= -1.0f;
        }
    }
    else if (-window.GetScreenWidth() * 0.5f >= transform.position.x - transform.size.x * 0.5f)
    {
        if (direction.x < 0.0f)
        {
            direction.x *= -1.0f;
        }
    }
    else if (window.GetScreenHeight() * 0.5f <= transform.position.y + transform.size.y * 0.5f)
    {
        if (direction.y > 0.0f)
        {
            direction.y *= -1.0f;
        }
    }
    else if (-window.GetScreenHeight() * 0.5f >= transform.position.y - transform.size.y * 0.5f)
    {
        if (direction.y < 0.0f)
        {
            direction.y *= -1.0f;
        }
    }
}

} // namespace Pong