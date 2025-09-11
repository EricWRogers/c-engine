#pragma once
#include <vector>
#include <Canis/Math.hpp>

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
        Vector2 position;
        Vector2 originOffset;
        float   depth;
        Vector2 rotation;
        Vector2 size;
        Vector4 color;
        Vector2 uv;
    };
}
