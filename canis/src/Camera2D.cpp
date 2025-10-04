#include <Canis/Camera2D.hpp>

namespace Canis
{
    Camera2D::Camera2D() : m_position(0.0f, 0.0f),
                           m_scale(8.0f),
                           m_needsMatrixUpdate(true),
                           m_screenWidth(500),
                           m_screenHeight(500)
    {
        m_cameraMatrix.Identity();
        m_view.Identity();
        m_projection.Identity();
    }

    Camera2D::~Camera2D()
    {
    }

    void Camera2D::Init(int screenWidth, int screenHeight)
    {
        m_screenWidth = screenWidth;
        m_screenHeight = screenHeight;
        m_projection.Orthographic(0.0f, (float)m_screenWidth, 0.0f, (float)m_screenHeight, 0.0f, 100.0f);
        SetPosition(Vector2((float)m_screenWidth / 2, (float)m_screenHeight / 2));
    }

    void Camera2D::Update()
    {
        if (m_needsMatrixUpdate)
        {
            m_view.Identity();
            m_view.Translate(Vector3(-m_position.x + m_screenWidth / 2, -m_position.y + m_screenHeight / 2, 0.0f));
            m_view.Scale(Vector3(m_scale, m_scale, 0.0f));

            m_cameraMatrix = m_projection * m_view;

            m_needsMatrixUpdate = false;
        }
    }
} // end of Canis namespace