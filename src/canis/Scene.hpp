#pragma once
#include <vector>
#include "Entity.hpp"
#include "ScriptManager.hpp"

namespace Canis
{
class Scene
{
private:
    std::vector<Entity> m_entities = {};
    std::vector<std::vector<Script>> m_scripts = {};
public:

    Entity CreateEntity() {
        Entity entity(m_entities.size(), this);
        m_entities.push_back(entity);
        m_scripts.push_back(std::vector<Script>());
        return entity;
    }

    void Update(float _deltaTime) {
        for (Entity entity : m_entities)
            if (entity.id != -1)
                for (Script script : m_scripts[entity.id])
                    script.Update(_deltaTime);
    }

    void DestroyEntity(Entity &_entity) {
        for (Script script : m_scripts[_entity.id])
            script.Destroy();
        
        m_entities[_entity.id].id = -1;
        _entity.id = -1;
    }

    void Destroy() {
        for (Entity entity : m_entities)
            DestroyEntity(entity);
    }

    std::vector<Script>& EntityScripts(Entity &_entity)
    {
        return m_scripts[_entity.id];
    }
};
}