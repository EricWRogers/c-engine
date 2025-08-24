#include <Canis/Scene.hpp>
#include <Canis/Entity.hpp>

namespace Canis
{
    void Scene::Init(App *_app)
    {
        app = _app;
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
}
