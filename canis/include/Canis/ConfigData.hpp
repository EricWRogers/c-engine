#pragma once
#include <map>
#include <string>
#include <vector>
#include <functional>
#include <yaml-cpp/yaml.h>

namespace Canis
{
class App;
class Entity;
class Editor;
class ScriptableEntity;

using PropertySetter = std::function<void(YAML::Node&, void*)>;
using PropertyGetter = std::function<YAML::Node(void*)>;

struct PropertyRegistry {
    std::map<std::string, PropertySetter> setters;
    std::map<std::string, PropertyGetter> getters;
    std::vector<std::string> propertyOrder;
};

struct ScriptConf {
    std::string name;
    PropertyRegistry registry;
    std::function<ScriptableEntity*(Entity&, bool)> Construct = nullptr;
    std::function<void(Entity&)> Add = nullptr;
    std::function<bool(Entity&)> Has = nullptr;
    std::function<void(Entity&)> Remove = nullptr;
    std::function<void*(Entity&)> Get = nullptr;
    std::function<void(YAML::Node &_node, Entity &_entity)> Encode = nullptr;
    std::function<void(YAML::Node &_node, Entity &_entity, bool _callCreate)> Decode = nullptr;
    std::function<void(Editor&, Entity&, const ScriptConf&)> DrawInspector = nullptr;
    //std::unordered_map<std::string, std::function<void>> exposedFunctions;
};

struct InspectorItemRightClick {
    std::string name;
    std::function<void(App&, Editor&, Entity&, std::vector<ScriptConf>&)> Func = nullptr;
};
} // namespace Canis
