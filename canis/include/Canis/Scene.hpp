#pragma once
#include <vector>
#include <string>
#include <Canis/Debug.hpp>

namespace Canis
{
    class App;
    class Window;
    class InputManager;
    class Entity;
    class System;

    class Scene
    {
    public:
        App* app = nullptr;
        
        void Init(App *_app, Window *_window, InputManager *_inputManger);
        void Update(float _deltaTime);
        void Render(float _deltaTime);
        void Unload();
        void Load();

        Window& GetWindow() { return *m_window; }
        InputManager& GetInputManager() { return *m_inputManager; }


        Entity* CreateEntity(std::string _name = "", std::string _tag = "");
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
            Debug::Log("CreateRenderSystem");
            System *s = new T();

            m_renderSystems.push_back(s);
            ReadySystem(s);
        }

        std::vector<Entity*>& GetEntities() { return m_entities; }
    private:
        Window *m_window;
        InputManager *m_inputManager;
        std::vector<Entity*>  m_entities = {};
        std::vector<System*> m_systems = {};
        std::vector<System*>  m_updateSystems = {};
        std::vector<System*>  m_renderSystems = {};

        void ReadySystem(System *_system);
    };
}
