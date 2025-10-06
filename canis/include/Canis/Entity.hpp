#pragma once
#include <vector>
#include <Canis/Math.hpp>
#include <Canis/AssetHandle.hpp>

namespace Canis
{
    class Scene;
    class ScriptableEntity;

    
    class Entity
    {
    friend Scene;
    private:
        std::vector<ScriptableEntity *> m_scriptComponents = {};
    public:
        int id;
        Scene *scene;
        bool active = true;
        

        template <typename T>
        T *AddScript()
        {
            T *scriptableEntity = new T(*this);

            // might check if the entity already has script

            m_scriptComponents.push_back((ScriptableEntity*)scriptableEntity);
            scriptableEntity->Create();

            return scriptableEntity;
        }

        template <typename T>
        T *GetScript()
        {
            T *scriptableEntity = nullptr;

            for (ScriptableEntity *sc : m_scriptComponents)
            {
                if ((scriptableEntity = dynamic_cast<T *>(sc)) != nullptr)
                {
                    return scriptableEntity;
                }
            }

            return scriptableEntity;
        }
    };

    class ScriptableEntity
    {
    friend Scene;
    private:
        bool m_onReadyCalled = false;
    public:        
        ScriptableEntity(Canis::Entity& _entity) : entity(_entity) {}

        Canis::Entity& entity;
        virtual void Create() {}
        virtual void Ready() {}
        virtual void Destroy() {}
        virtual void Update(float _dt) {}
    };

    class Sprite2D : public ScriptableEntity
    {
    public:
        Sprite2D(Canis::Entity& _entity) : Canis::ScriptableEntity(_entity) {}

        Vector2 position = Vector2(0.0f);
        Vector2 originOffset = Vector2(0.0f);
        float   depth = 0.0f;
        float rotation = 0.0f;
        Vector2 size = Vector2(32.0f);
        Color   color = Color(1.0f);
        Vector4 uv = Vector4(0.0f, 0.0f, 1.0f, 1.0f);
        TextureHandle textureHandle;
    };

    class Camera2D : public ScriptableEntity
    {
    public:
        Camera2D(Canis::Entity& _entity);
        ~Camera2D();

        void Create();

        void Update(float _dt);

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
        float m_scale = 1.0f;
        Vector2 m_position = Vector2(0.0f);
        Matrix4 m_cameraMatrix;
        Matrix4 m_view;
        Matrix4 m_projection;
    };
}
