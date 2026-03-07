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
void Paddle::Ready() {}
void Paddle::Destroy() {}

void Paddle::Update(float _dt)
{
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
    
    Vector2 delta = direction * speed * Time::DeltaTime();
    transform.position += delta;

    if (Entity* e = entity.scene->FindEntityWithName("Ball"))
    {
        RectTransform& ballTransform = *CANIS_GET_SCRIPT(e, RectTransform);

        float distance = glm::distance(transform.GetPosition(), ballTransform.GetPosition());

        if (distance < (ballTransform.size.x * ballTransform.scale.x * 0.5f) + (transform.size.x * transform.scale.x * 0.5f))
        {
            Ball& ball = *CANIS_GET_SCRIPT(e, Ball);

            ball.direction = glm::normalize(ballTransform.GetPosition() - transform.GetPosition());

        }
    }
}

void Paddle::EditorInspectorDraw() {}

} // namespace Pong
