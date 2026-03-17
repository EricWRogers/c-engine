#include "../../include/Pong/Ball.hpp"

using namespace Canis;

namespace Pong
{
    void Ball::Create()
    {
        direction = glm::normalize(Vector2(-1.0f, -1.0f)); 
    }

    void Ball::Ready()
    {
        m_transform = entity.GetComponent<RectTransform>();
        m_sprite = entity.GetComponent<Sprite2D>();
        direction = glm::normalize(Vector2(-1.0f, 0.0f));
    }

    void Ball::Destroy() {}

    void Ball::Update(float _dt)
    {
        m_transform = entity.GetComponent<RectTransform>();
        if (m_transform == nullptr)
            return;

        Vector2 delta = direction * speed * _dt;
        m_transform->position += delta;
        m_transform->rotation += DEG2RAD * 5000.0f * _dt;

        CheckWalls();
    }

    void Ball::EditorInspectorDraw() {}

    void Ball::CheckWalls()
    {
        Canis::Window &window = entity.scene->GetWindow();
        if (m_transform == nullptr)
            return;

        if (window.GetScreenWidth() * 0.5f <= m_transform->position.x + m_transform->size.x * 0.5f)
        {
            if (direction.x > 0.0f)
            {
                direction.x *= -1.0f;
            }
        }
        else if (-window.GetScreenWidth() * 0.5f >= m_transform->position.x - m_transform->size.x * 0.5f)
        {
            if (direction.x < 0.0f)
            {
                direction.x *= -1.0f;
            }
        }
        else if (window.GetScreenHeight() * 0.5f <= m_transform->position.y + m_transform->size.y * 0.5f)
        {
            if (direction.y > 0.0f)
            {
                direction.y *= -1.0f;
            }
        }
        else if (-window.GetScreenHeight() * 0.5f >= m_transform->position.y - m_transform->size.y * 0.5f)
        {
            if (direction.y < 0.0f)
            {
                direction.y *= -1.0f;
            }
        }
    }

} // namespace Pong
