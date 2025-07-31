#include <Canis/Scene.hpp>

namespace Canis
{
    Entity Scene::CreateEntity()
    {
        Entity* entity = new Entity();
        entity->id = m_entities.size();
        entity->scene = this;
        m_entities.push_back(entity);
        return *entity;
    }
}