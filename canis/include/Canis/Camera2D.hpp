#pragma once
#include <Canis/Math.hpp>

namespace Canis
{
    class Camera2D
    {
    public:
        Camera2D();
        ~Camera2D();

        void Init(int screenWidth, int screenHeight);

        void Update();

        void SetPosition(const Vector2 &newPosition)
        {
            m_position = newPosition;
            m_needsMatrixUpdate = true;
        }
        void SetScale(float newScale)
        {
            m_scale = newScale;
            m_needsMatrixUpdate = true;
        }

        Vector2 GetPosition() { return m_position; }
        Matrix4 GetCameraMatrix() { return m_cameraMatrix; }
        Matrix4 GetViewMatrix() { return m_view; }
        Matrix4 GetProjectionMatrix() { return m_projection; }
        float GetScale() { return m_scale; }

    private:
        int m_screenWidth, m_screenHeight;
        bool m_needsMatrixUpdate;
        float m_scale;
        Vector2 m_position;
        Matrix4 m_cameraMatrix;
        Matrix4 m_view;
        Matrix4 m_projection;
    };
} // end of Canis namespace