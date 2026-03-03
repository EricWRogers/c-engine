#include "../../include/Pong/Ball.hpp"

using namespace Canis;

namespace Pong
{
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

    void Ball::EditorInspectorDraw() {}

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