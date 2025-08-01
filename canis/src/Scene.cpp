#include <Canis/Scene.hpp>
#include <SDL3/SDL_log.h>

namespace Canis
{
    void Scene::Init()
    {

    }

    void Scene::Update(float _deltaTime)
    {
        SDL_Log("Scene::Update");
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

    void Destroy(uint _id)
    {

    }
}