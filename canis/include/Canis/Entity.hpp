#pragma once
#include <vector>

namespace Canis
{
    class Scene;

    class ScriptableEntity
    {
    public:
        virtual void OnCreate() {}
        virtual void OnReady() {}
        virtual void OnDestroy() {}
        virtual void OnUpdate(float _dt) {}
    };

    class Entity
    {
    private:
    public:
        int id;
        Scene *scene;
        std::vector<ScriptableEntity *> scriptComponents = {};

        template <typename T>
        T *GetScript()
        {
            T *scriptableEntity = nullptr;

            for (ScriptableEntity *sc : scriptComponents)
            {
                if ((scriptableEntity = dynamic_cast<T *>(sc)) != nullptr)
                {
                    return scriptableEntity;
                }
            }

            return scriptableEntity;
        }
    };
}