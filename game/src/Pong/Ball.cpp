#include "../../include/Pong/Ball.hpp"

#include <Canis/App.hpp>
#include <Canis/Time.hpp>
#include <Canis/Window.hpp>
#include <Canis/InputManager.hpp>

#include <ConfigHelper.hpp>

#include <map>

using namespace Canis;

namespace Pong
{
ScriptConf conf = {};

void RegisterBallScript(Canis::App& _app)
{
    REGISTER_PROPERTY(Pong::Ball, direction, Vector2);
    REGISTER_PROPERTY(Pong::Ball, speed, float);
    REGISTER_PROPERTY(Pong::Ball, randomRotation, float);

    conf.DEFAULT_NAME(Pong::Ball);
    conf.DEFAULT_ADD_AND_REQUIRED(Pong::Ball, Canis::RectTransform, Canis::Sprite2D);
    conf.DEFAULT_HAS(Pong::Ball);
    conf.DEFAULT_REMOVE(Pong::Ball);
    conf.DEFAULT_GET(Pong::Ball);
    conf.DEFAULT_ENCODE(Pong::Ball);
    conf.DEFAULT_DECODE(Pong::Ball);

    conf.DrawInspector = [](Editor &_editor, Entity &_entity, const ScriptConf &_conf) -> void
    {
        Ball *ball = nullptr;
        if ((ball = _entity.GetScript<Ball>()) != nullptr)
        {
            ImGui::InputFloat2(("direction##" + _conf.name).c_str(), &ball->direction.x, "%.3f");
            ImGui::InputFloat(("speed##" + _conf.name).c_str(), &ball->speed);
            ImGui::InputFloat(("randomRotation##" + _conf.name).c_str(), &ball->randomRotation);
        }
    };
    
    _app.RegisterScript(conf);
}

DEFAULT_UNREGISTER_SCRIPT(Ball)

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