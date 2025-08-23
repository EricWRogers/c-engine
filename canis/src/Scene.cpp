#include <Canis/Scene.hpp>

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

    void Destroy(int _id)
    {

    }
}
