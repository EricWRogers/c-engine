#pragma once
#include <vector>

namespace Canis
{
    class App;
    class Window;
    class Entity;
    class System;

    class Scene
    {
    public:
        App* app = nullptr;
        
        void Init(App *_app, Window *_window);
        void Update(float _deltaTime);
        void Unload();

        Entity* CreateEntity();
        Entity* GetEntity(int _id);

        void Destroy(int _id);

        template <typename T>
        T *GetSystem()
        {
            T *castedSystem = nullptr;

            for (System *s : m_systems)
                if ((castedSystem = dynamic_cast<T *>(s)) != nullptr)
                    return castedSystem;

            return nullptr;
        }

        template <typename T>
        void CreateSystem()
        {
            System *s = new T();

            m_updateSystems.push_back(s);
            ReadySystem(s);
        }

        template <typename T>
        void CreateRenderSystem()
        {
            System *s = new T();

            m_renderSystems.push_back(s);
            ReadySystem(s);
        }
    private:
        Window *m_window;
        std::vector<Entity*>  m_entities = {};
        std::vector<System *> m_systems = {};
        std::vector<System*>  m_updateSystems = {};
        std::vector<System*>  m_renderSystems = {};

        void ReadySystem(System *_system);
    };
}
