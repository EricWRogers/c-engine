#pragma once
#include <Canis/UUID.hpp>

#include <string>
#include <vector>

namespace YAML
{
    class Node;
}

namespace Canis
{
    
    class App;
    class Window;
    class InputManager;
    class Entity;
    class System;
    struct ScriptConf;

    class Scene
    {
    public:
        App* app = nullptr;
        
        void Init(App *_app, Window *_window, InputManager *_inputManger, std::string _path);
        void Update(float _deltaTime);
        void Render(float _deltaTime);
        void Unload();
        
        void Save(std::vector<ScriptConf>& _scriptRegistry);
        YAML::Node EncodeScene(std::vector<ScriptConf>& _scriptRegistry);
        YAML::Node EncodeEntity(std::vector<ScriptConf>& _scriptRegistry, Entity &_entity);

        void Load(std::vector<ScriptConf>& _scriptRegistry);
        void LoadSceneNode(std::vector<ScriptConf>& _scriptRegistry, YAML::Node &_root);
        void LoadEntityNodes(std::vector<ScriptConf>& _scriptRegistry, YAML::Node &_entities, bool _copyUUID = true);
        Canis::Entity& DecodeEntity(std::vector<ScriptConf>& _scriptRegistry, YAML::Node _node, bool _copyUUID = true);
        void GetEntityAfterLoad(Canis::UUID _uuid, Canis::Entity* &_variable);

        Window& GetWindow() { return *m_window; }
        InputManager& GetInputManager() { return *m_inputManager; }


        Entity* CreateEntity(std::string _name = "", std::string _tag = "");
        Entity* GetEntity(int _id);
        Entity* GetEntityWithUUID(Canis::UUID _uuid);
        Entity* FindEntityWithName(std::string _name);

        Entity* GetEntityWithTag(std::string _tag);
        std::vector<Entity*> GetEntitiesWithTag(std::string _tag);

        void Destroy(int _id);
        void Destroy(Entity& _entity);

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

        std::vector<Entity*>& GetEntities() { return m_entities; }
    private:
        std::string m_name = "main";
        std::string m_path = "assets/scenes/main.scene";
        Window *m_window;
        InputManager *m_inputManager;

        std::vector<Entity*> m_entities = {};
        std::vector<System*> m_systems = {};
        std::vector<System*> m_updateSystems = {};
        std::vector<System*> m_renderSystems = {};
        std::vector<int> m_entitiesToReady = {};
        std::vector<int> m_entitiesToDestroy = {};
        bool m_isUpdating = false;

        // this is used when duplicating entity
        std::unordered_map<UUID, UUID> m_targetUUIDNewUUID;

        struct EntityConnectInfo {
            Canis::UUID targetUUID;
            Canis::Entity** variable;
        };

        std::vector<EntityConnectInfo> m_entityConnectInfo = {};

        void QueueEntityForReady(int _id);
        void DestroyNow(int _id);
        void ReadySystem(System *_system);
    };
}
