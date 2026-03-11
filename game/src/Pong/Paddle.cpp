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
    m_transform = CANIS_GET_COMPONENT(entity, RectTransform);
    m_sprite = CANIS_GET_COMPONENT(entity, Sprite2D);
}
void Paddle::Destroy() {}

void Paddle::Update(float _dt)
{
    m_transform = CANIS_GET_COMPONENT(entity, RectTransform);
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
        RectTransform& ballTransform = *CANIS_GET_COMPONENT(e, RectTransform);

        float distance = glm::distance(m_transform->GetPosition(), ballTransform.GetPosition());

        if (distance < (ballTransform.size.x * ballTransform.scale.x * 0.5f) + (m_transform->size.x * m_transform->scale.x * 0.5f))
        {
            Ball& ball = *CANIS_GET_SCRIPT(e, Ball);

            ball.direction = glm::normalize(ballTransform.GetPosition() - m_transform->GetPosition());

        }
    }
}

void Paddle::EditorInspectorDraw() {}

} // namespace Pong
