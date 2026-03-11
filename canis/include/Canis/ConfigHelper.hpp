#pragma once
#include <Canis/ConfigData.hpp>
#include <Canis/Entity.hpp>
#include <Canis/Scene.hpp>
#include <Canis/Editor.hpp>
#include <Canis/Yaml.hpp>

#include <algorithm>

#include <imgui.h>
#include <imgui_stdlib.h>

using namespace Canis;

#define DEFAULT_NAME(type) \
    name = type::ScriptName \

#define DEFAULT_ADD(type) \
    Add = [](Entity &_entity) -> void { (void)CANIS_ADD_SCRIPT(_entity, type); } \

#define DEFAULT_CONSTRUCT(type) \
    Construct = [](Entity &_entity, bool _callCreate) -> ScriptableEntity* { return _entity.AttachScript(type::ScriptName, new type(_entity), _callCreate); } \

template<typename... Ts>
inline void AddRequiredScripts(Entity& _entity)
{
    (_entity.scene->app->AddRequiredScript(_entity, Ts::ScriptName), ...);
}

#define DEFAULT_ADD_AND_REQUIRED(type, ...)                               \
    Add = [](Entity &_entity)   -> void                                   \
    {                                                                     \
        AddRequiredScripts<__VA_ARGS__>(_entity);                         \
        (void)CANIS_ADD_SCRIPT(_entity, type);                            \
    }                                                                     \

#define DEFAULT_HAS(type) \
    Has = [](Entity &_entity) -> bool { return CANIS_HAS_SCRIPT(_entity, type); } \

#define DEFAULT_REMOVE(type) \
    Remove = [](Entity &_entity) -> void { CANIS_REMOVE_SCRIPT(_entity, type); } \

#define DEFAULT_GET(type) \
    Get = [](Entity& _entity) -> void* { return (void*)CANIS_GET_SCRIPT(_entity, type); } \

#define DECODE(node, component, property) \
    component.property = node[#property].as<decltype(component.property)>(component.property); \

#define DEFAULT_REGISTER_SCRIPT(type)           \
void Register##type##Script(Canis::App& _app)   \
{                                               \
    _app.RegisterScript(conf);                  \
}

#define DEFAULT_UNREGISTER_SCRIPT(_conf, type)   \
void UnRegister##type##Script(Canis::App& _app)  \
{                                                \
    _app.UnregisterScript(_conf);                \
}

#define REGISTER_PROPERTY(config, component, property, type)                                      \
{                                                                                                   \
    config.registry.setters[#property] = [](YAML::Node &node, void *componentPtr) {             	        \
        static_cast<component *>(componentPtr)->property = node.as<type>();                         \
    };                                                                                              \
                                                                                                    \
    config.registry.getters[#property] = [](void *componentPtr) -> YAML::Node {                     		\
        return YAML::Node(static_cast<component *>(componentPtr)->property);                        \
    };                                                                                         		\
                                                                                               		\
    config.registry.propertyOrder.push_back(#property);                                             		\
}

#define UNREGISTER_PROPERTY(registry, component, property)                          \
{                                                                                   \
    registry.setters.erase(#property);                                              \
    registry.getters.erase(#property);                                              \
                                                                                    \
    auto &order = registry.propertyOrder;                                           \
    order.erase(std::remove(order.begin(), order.end(), std::string(#property)),    \
                order.end());                                                       \
}                                                                                   \

inline void EncodeComponent(PropertyRegistry& _registry, YAML::Node &_node, Entity &_entity, const std::string& _scriptName)
{
    if (ScriptableEntity* component = _entity.GetScript(_scriptName))
    {
        YAML::Node comp;

        for (const auto &propertyName : _registry.propertyOrder)
        {
            comp[propertyName] = _registry.getters[propertyName](component);
        }

        _node[_scriptName] = comp;
    }
}

#define DEFAULT_ENCODE(config, type) \
    Encode = [](YAML::Node &_node, Entity &_entity) -> void { EncodeComponent(config.registry, _node, _entity, type::ScriptName); } \

inline void DecodeComponent(PropertyRegistry& _registry, YAML::Node &_node, Canis::Entity &_entity, const std::string& _scriptName, bool _callCreate)
{
    if (auto componentNode = _node[_scriptName])
    {
        ScriptableEntity* script = _entity.AddScript(_scriptName, false);
        if (script == nullptr)
            return;

        for (const auto &[propertyName, setter] : _registry.setters)
        {
            if (componentNode[propertyName])
            {
                YAML::Node propertyNode = componentNode[propertyName];
                setter(propertyNode, script);
            }
        }
        if (_callCreate)
            script->Create();
    }
}

#define DEFAULT_DECODE(config, type) \
    Decode = [](YAML::Node &_node, Entity &_entity, bool _callCreate) -> void { DecodeComponent(config.registry, _node, _entity, type::ScriptName, _callCreate); } \

#define DEFAULT_CONFIG(config, type)                \
{                                                   \
    config.DEFAULT_NAME(type);                      \
    config.DEFAULT_CONSTRUCT(type);                 \
    config.DEFAULT_ADD(type);                       \
    config.DEFAULT_HAS(type);                       \
    config.DEFAULT_REMOVE(type);                    \
    config.DEFAULT_ENCODE(config, type);            \
    config.DEFAULT_DECODE(config, type);            \
}

#define DEFAULT_CONFIG_AND_REQUIRED(config, type, ...)  \
{                                                       \
    config.DEFAULT_NAME(type);                          \
    config.DEFAULT_CONSTRUCT(type);                     \
    config.Add = [](Entity &_entity) -> void            \
    {                                                   \
        AddRequiredScripts<__VA_ARGS__>(_entity);       \
        (void)CANIS_ADD_SCRIPT(_entity, type);          \
    };                                                  \
    config.DEFAULT_HAS(type);                           \
    config.DEFAULT_REMOVE(type);                        \
    config.DEFAULT_ENCODE(config, type);                \
    config.DEFAULT_DECODE(config, type);                \
}

/*#define CHECK_SCRIPTABLE_ENTITY(type) \
    Canis::Entity e; \
    type v(e); \
    if ((void*)dynamic_cast<Canis::ScriptableEntity*>(&v) == nullptr) { \
        Debug::Error("%s does NOT publicly inherit ScriptableEntity!", type_name<type>().data()); \
    } else { \
        Debug::Log("I Checked!"); \
    }*/
