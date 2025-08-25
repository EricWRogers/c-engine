#include <Canis/Scene.hpp>
#include <Canis/Entity.hpp>
#include <Canis/System.hpp>

namespace Canis
{
    void Scene::Init(App *_app, Window *_window)
    {
        app = _app;
        m_window = _window;
    }

    void Scene::Update(float _deltaTime)
    {
        for (Entity* e : m_entities)
        {
            for (ScriptableEntity* se : e->m_scriptComponents)
            {
                se->OnUpdate(_deltaTime);
            }
        }
    }

    void Scene::Unload()
    {
        for (Entity* e : m_entities)
        {
            for (ScriptableEntity* se : e->m_scriptComponents)
            {
                se->OnDestroy();
            }
        }

        for (Entity* e : m_entities)
        {
            for (ScriptableEntity* se : e->m_scriptComponents)
            {
                delete se;
            }
        }

        for (Entity* e : m_entities)
        {
            delete e;
        }

        m_entities.clear();
    }

    Entity* Scene::CreateEntity()
    {
        Entity* entity = new Entity();
        entity->id = m_entities.size();
        entity->scene = this;
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

    void Destroy(int _id)
    {

    }

    void Scene::ReadySystem(System *_system)
    {
        _system->scene = this;
        _system->window = m_window;
        //_system->inputManager = inputManager;
        //_system->time = m_time;
        //_system->camera = camera;
        m_systems.push_back(_system);
    }
}
