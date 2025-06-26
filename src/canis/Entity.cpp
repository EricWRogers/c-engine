#include "Entity.hpp"
#include "Scene.hpp"
#include "ScriptManager.hpp"

namespace Canis
{
    void Entity::AddScript(std::string _className)
    {
        std::vector<Script>& scripts = scene->EntityScripts(*this);
        scripts.push_back(ScriptManager::LoadScript(_className));
        scripts[scripts.size()-1].Start();
    }

    void Entity::Destroy()
    {
        scene->DestroyEntity(*this);
    }
}