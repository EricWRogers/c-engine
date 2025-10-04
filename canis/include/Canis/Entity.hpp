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
            T *scriptableEntity = new T();
            scriptableEntity->entity = this;

            // might check if the entity already has script

            m_scriptComponents.push_back((ScriptableEntity*)scriptableEntity);
            scriptableEntity->OnCreate();

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
        Canis::Entity* entity = nullptr;
        virtual void OnCreate() {}
        virtual void OnReady() {}
        virtual void OnDestroy() {}
        virtual void OnUpdate(float _dt) {}
    };

    class Sprite2D : public ScriptableEntity
    {
    public:
        Vector2 position = Vector2(0.0f);
        Vector2 originOffset = Vector2(0.0f);
        float   depth = 0.0f;
        float rotation = 0.0f;
        Vector2 size = Vector2(32.0f);
        Color   color = Color(1.0f);
        Vector4 uv = Vector4(0.0f, 0.0f, 1.0f, 1.0f);
        TextureHandle textureHandle;
    };
}
