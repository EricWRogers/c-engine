#include "../../include/Pong/Paddle.hpp"

#include <Canis/App.hpp>
#include <Canis/Time.hpp>
#include <Canis/Scene.hpp>
#include <Canis/Window.hpp>
#include <Canis/Editor.hpp>
#include <Canis/InputManager.hpp>

#include "../../include/Pong/Ball.hpp"

#include <algorithm>

using namespace Canis;

namespace Pong
{
void Paddle::Create() {}
void Paddle::Ready()
{
    m_transform = entity.HasComponent<RectTransform>() ? &entity.GetComponent<RectTransform>() : nullptr;
    m_sprite = entity.HasComponent<Sprite2D>() ? &entity.GetComponent<Sprite2D>() : nullptr;
}
void Paddle::Destroy() {}

void Paddle::Update(float _dt)
{
    m_transform = entity.HasComponent<RectTransform>() ? &entity.GetComponent<RectTransform>() : nullptr;
    if (m_transform == nullptr)
        return;

    InputManager& inputManager = entity.scene->GetInputManager();

    playerNum = std::clamp(playerNum, 1, 2);

    unsigned int up = (playerNum == 1) ? Canis::Key::W : Canis::Key::UP;
    unsigned int down = (playerNum == 1) ? Canis::Key::S : Canis::Key::DOWN;
    
    if (inputManager.GetKey(up))
        direction.y = 1.0f;
    else if (inputManager.GetKey(down))
        direction.y = -1.0f;
    else
        direction.y = 0.0f;
    
    Vector2 delta = direction * speed * _dt;
    m_transform->position += delta;

    if (Entity* e = entity.scene->FindEntityWithName("Ball"))
    {
        if (!e->HasComponent<RectTransform>())
            return;

        RectTransform& ballTransform = e->GetComponent<RectTransform>();

        float distance = glm::distance(m_transform->GetPosition(), ballTransform.GetPosition());

        if (distance < (ballTransform.size.x * ballTransform.scale.x * 0.5f) + (m_transform->size.x * m_transform->scale.x * 0.5f))
        {
            if (!e->HasScript(Ball::ScriptName))
                return;

            Ball& ball = e->GetScript<Ball>();

            ball.direction = glm::normalize(ballTransform.GetPosition() - m_transform->GetPosition());

        }
    }
}

void Paddle::EditorInspectorDraw() {}

} // namespace Pong
