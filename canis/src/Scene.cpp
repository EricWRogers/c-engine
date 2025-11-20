#include <Canis/Scene.hpp>
#include <Canis/Debug.hpp>
#include <Canis/Entity.hpp>
#include <Canis/System.hpp>
#include <Canis/Window.hpp>
#include <Canis/ECS/Systems/SpriteRenderer2DSystem.hpp>

namespace Canis
{
    void Scene::Init(App *_app, Window *_window, InputManager *_inputManger, std::string _path)
    {
        app = _app;
        m_window = _window;
        m_inputManager = _inputManger;
        m_path = _path;
    }

    void Scene::Update(float _deltaTime)
    {
        // TODO: when an extity is created add its id to a list
        // call ready just on that list then clear it for better performance
        for (Entity* e : m_entities)
        {
            if (e == nullptr)
                continue;

            for (ScriptableEntity* se : e->m_scriptComponents)
            {
                if (se->m_onReadyCalled)
                    continue;

                se->Ready();
                se->m_onReadyCalled = true;
            }
        }

        for (Entity* e : m_entities)
        {
            if (e == nullptr)
                continue;

            for (ScriptableEntity* se : e->m_scriptComponents)
            {
                se->Update(_deltaTime);
            }
        }
    }

    void Scene::Render(float _deltaTime)
    {
        //Canis::Debug::Log("Render Update %i", m_renderSystems.size());
        for (System* renderer : m_renderSystems)
        {
            //Canis::Debug::Log("Render Update for");
            renderer->Update();
        }
    }

    void Scene::Unload()
    {
        for (Entity* e : m_entities)
        {
            if (e == nullptr)
                continue;

            for (ScriptableEntity* se : e->m_scriptComponents)
            {
                se->Destroy();
            }
        }

        for (Entity* e : m_entities)
        {
            if (e == nullptr)
                continue;

            for (ScriptableEntity* se : e->m_scriptComponents)
            {
                delete se;
            }
        }

        for (Entity* e : m_entities)
        {
            if (e == nullptr)
                continue;
                
            delete e;
        }

        m_entities.clear();

        for (System* system : m_systems)
        {
            system->OnDestroy();
        }

        for (System* system : m_systems)
        {
            delete system;
        }

        m_systems.clear();
        m_updateSystems.clear();
        m_renderSystems.clear();
    }

    void Scene::Load(std::vector<ScriptConf>& _scriptRegistry)
    {
        YAML::Node root = YAML::LoadFile(m_path);

        if (!root)
        {
            Debug::FatalError("Scene not found: %s", m_path.c_str());
        }
        
        LoadSceneNode(_scriptRegistry, root);
    }

    void Scene::LoadSceneNode(std::vector<ScriptConf>& _scriptRegistry, YAML::Node &_root)
    {
        CreateRenderSystem<Canis::SpriteRenderer2DSystem>();
        
        for (System* system : m_systems)
        {
            system->Create();
        }

        for (System* system : m_systems)
        {
            system->Ready();
        }

        auto environment = _root["Environment"]; 

        if (environment)
        {
            m_window->SetClearColor(
                environment["ClearColor"].as<Vector4>(Vector4(0.05f, 0.05f, 0.05f, 1.0f))
            );
        }

        auto entities = _root["Entities"];

        m_entityConnectInfo.clear();
        std::vector<Canis::Entity*> newEntitys = {};

        if (entities)
        {
            for (auto e : entities)
            {
                newEntitys.push_back(&DecodeEntity(_scriptRegistry, e));
            }

            for (auto eci : m_entityConnectInfo)
            {
                (*eci.variable) = GetEntityWithUUID(eci.targetUUID);
            }

            for (auto e : newEntitys)
            {
                for (ScriptableEntity* se : e->m_scriptComponents)
                {
                    if (se)
                    {
                        se->Create();
                    }
                }
            }
        }
    }

    Canis::Entity& Scene::DecodeEntity(std::vector<ScriptConf>& _scriptRegistry, YAML::Node _node, bool _copyUUID)
    {
        Entity& entity = *CreateEntity();
        
        if (_copyUUID)
            entity.uuid = _node["Entity"].as<Canis::UUID>(0);
        
        entity.name = _node["Name"].as<std::string>("");
        entity.tag = _node["Tag"].as<std::string>("");

        for (int i = 0; i < _scriptRegistry.size(); i++)
        {
            if (_scriptRegistry[i].Decode)
            {
                _scriptRegistry[i].Decode(_node, entity, false);
            }
        }

        return entity;
    }

    void Scene::GetEntityAfterLoad(Canis::UUID _uuid, Canis::Entity* &_variable)
    {
        if (_uuid == 0)
        {
            _variable = nullptr;
            return;
        }

        EntityConnectInfo eci;
        eci.targetUUID = _uuid;
        eci.variable = &_variable;

        m_entityConnectInfo.push_back(eci);
    }

    void Scene::Save(std::vector<ScriptConf>& _scriptRegistry)
    {
        Debug::Log("Save Scene");
        
        YAML::Emitter out;

        out << EncodeScene(_scriptRegistry);

        if (m_path.size() > 0)
        {
            std::ofstream fout(m_path);
            fout << out.c_str();
        }
        else
        {
            std::ofstream fout(m_name);
            fout << out.c_str();
        }
    }

    YAML::Node Scene::EncodeScene(std::vector<ScriptConf>& _scriptRegistry)
    {
        YAML::Node node;

        YAML::Node environment;
        environment["ClearColor"] = m_window->GetClearColor();
        node["Environment"] = environment;

        YAML::Node entities = YAML::Node(YAML::NodeType::Sequence);

        for(Entity* entity : m_entities)
        {
            if (!entity)
                continue;

            entities.push_back(EncodeEntity(_scriptRegistry, *entity));
        }

        node["Entities"] = entities;

        return node;
    }

    YAML::Node Scene::EncodeEntity(std::vector<ScriptConf>& _scriptRegistry, Entity &_entity)
    {
        YAML::Node node;
        node["Entity"] = _entity.uuid;
        node["Name"] = _entity.name;
        node["Tag"] = _entity.tag;

        for (int i = 0; i < _scriptRegistry.size(); i++)
            if (_scriptRegistry[i].Encode)
                _scriptRegistry[i].Encode(node, _entity);
        
        return node;
    }

    Entity* Scene::CreateEntity(std::string _name, std::string _tag)
    {
        Entity* entity = new Entity();
        entity->id = m_entities.size();
        entity->scene = this;
        entity->name = _name;
        entity->tag = _tag;

        // TODO : handle better
        for (int i = 0; i < m_entities.size(); i++)
        {
            if (m_entities[i] == nullptr)
            {
                m_entities[i] = entity;
                return entity;
            }
        }
        
        m_entities.push_back(entity);
        return entity;
    }

    Entity* Scene::GetEntity(int _id)
    {
        if (_id > -1 && _id < m_entities.size())
            return m_entities[_id];
        
        // TODO : Handle Error
        return nullptr; 
    }

    Entity* Scene::GetEntityWithUUID(Canis::UUID _uuid)
    {
        for (int i = 0; i < m_entities.size(); i++)
            if (m_entities[i] != nullptr)
                if (m_entities[i]->uuid == _uuid)
                    return m_entities[i];

        
        // TODO : Handle Error
        return nullptr; 
    }

    Entity* Scene::FindEntityWithName(std::string _name)
    {
        for (Entity* entity : m_entities)
        {
            if (entity == nullptr)
                continue;
            
            if (entity->name == _name)
                return entity;
        }

        return nullptr;
    }

    void Scene::Destroy(int _id)
    {
        if (_id < 0 || m_entities.size() <= _id)
            return;
        
        Destroy(*m_entities[_id]);
    }

    void Scene::Destroy(Entity& _entity)
    {
        
        for (ScriptableEntity* se : _entity.m_scriptComponents)
        {
            se->Destroy();
        }

        
        for (ScriptableEntity* se : _entity.m_scriptComponents)
        {
            delete se;
        }

        _entity.m_scriptComponents.clear();

        delete m_entities[_entity.id];
        m_entities[_entity.id] = nullptr;
    }

    void Scene::ReadySystem(System *_system)
    {
        _system->scene = this;
        _system->window = m_window;
        _system->inputManager = m_inputManager;
        //_system->time = m_time;
        //_system->camera = camera;
        m_systems.push_back(_system);
    }
}
