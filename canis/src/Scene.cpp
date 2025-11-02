#include <Canis/Scene.hpp>
#include <Canis/Debug.hpp>
#include <Canis/Entity.hpp>
#include <Canis/System.hpp>

namespace Canis
{
    void Scene::Init(App *_app, Window *_window, InputManager *_inputManger)
    {
        app = _app;
        m_window = _window;
        m_inputManager = _inputManger;
    }

    void Scene::Update(float _deltaTime)
    {
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

    void Scene::Load()
    {
        for (System* system : m_systems)
        {
            system->Create();
        }

        for (System* system : m_systems)
        {
            system->Ready();
        }
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
