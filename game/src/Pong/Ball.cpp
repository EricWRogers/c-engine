#include "../../include/Pong/Ball.hpp"

#include <Canis/App.hpp>
#include <Canis/Time.hpp>
#include <Canis/Window.hpp>
#include <Canis/InputManager.hpp>

using namespace Canis;

namespace Pong
{

#define DEFAULT_NAME(type) \
    .name = #type \

#define DEFAULT_ADD(type) \
    .Add = [](Entity &_entity) -> void { _entity.AddScript<type>(); } \

#define DEFAULT_HAS(type) \
    .Has = [](Entity &_entity) -> bool { return (_entity.GetScript<type>() != nullptr); } \

#define DEFAULT_REMOVE(type) \
    .Remove = [](Entity &_entity) -> void { _entity.RemoveScript<type>(); } \

#define DECODE(node, component, property) \
    component.property = node[#property].as<decltype(component.property)>(component.property); \


ScriptConf ballConf = {
    DEFAULT_NAME(Pong::Ball),
    /*DEFAULT_ADD(Pong::Ball),*/
    DEFAULT_HAS(Pong::Ball),
    DEFAULT_REMOVE(Pong::Ball),
    .Add = [](Entity &_entity) -> void
    {
        _entity.scene->app->AddRequiredScript(_entity, "Canis::RectTransform");
        _entity.scene->app->AddRequiredScript(_entity, "Canis::Sprite2D");
        _entity.AddScript<Ball>();
    },
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
            DECODE(ballComponent, ball, direction)
            DECODE(ballComponent, ball, speed)
            DECODE(ballComponent, ball, randomRotation)
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
    direction = Vector2(-1.0f, 0.0f).Normalize();
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