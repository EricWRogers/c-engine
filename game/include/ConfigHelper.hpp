#pragma once

#include <Canis/Entity.hpp>
#include <Canis/Scene.hpp>
#include <Canis/Editor.hpp>

#include <algorithm>

using namespace Canis;

#define DEFAULT_NAME(type) \
    name = #type \

#define DEFAULT_ADD(type) \
    Add = [](Entity &_entity) -> void { _entity.AddScript<type>(); } \

template<typename... Ts>
inline void AddRequiredScripts(Entity& _entity)
{
    // Fold expression over the type pack Ts...
    (
        _entity.scene->app->AddRequiredScript(
            _entity,
            std::string(type_name<Ts>()) // convert string_view -> std::string
        ),
        ...
    );
}

#define DEFAULT_ADD_AND_REQUIRED(type, ...)                               \
    Add = [](Entity &_entity)   -> void                                   \
    {                                                                     \
        AddRequiredScripts<__VA_ARGS__>(_entity);                         \
        _entity.AddScript<type>();                                        \
    }                                                                     \

#define DEFAULT_HAS(type) \
    Has = [](Entity &_entity) -> bool { return (_entity.GetScript<type>() != nullptr); } \

#define DEFAULT_REMOVE(type) \
    Remove = [](Entity &_entity) -> void { _entity.RemoveScript<type>(); } \

#define DEFAULT_GET(type) \
    Get = [](Entity& _entity) -> void* { return (void*)_entity.GetScript<type>(); } \

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

template <typename ComponentType>
void EncodeComponent(PropertyRegistry& _registry, YAML::Node &_node, Entity &_entity)
{
    if (_entity.GetScript<ComponentType>())
    {
        ComponentType &component = *_entity.GetScript<ComponentType>();

        YAML::Node comp;

        for (const auto &propertyName : _registry.propertyOrder)
        {
            comp[propertyName] = _registry.getters[propertyName](&component);
        }

        _node[type_name<ComponentType>()] = comp;
    }
}

#define DEFAULT_ENCODE(config, type) \
    Encode = [](YAML::Node &_node, Entity &_entity) -> void { EncodeComponent<type>(config.registry, _node, _entity); } \

template <typename ComponentType>
void DecodeComponent(PropertyRegistry& _registry, YAML::Node &_node, Canis::Entity &_entity, bool _callCreate)
{
    if (auto componentNode = _node[std::string(type_name<ComponentType>())])
    {
        auto &script = *_entity.AddScript<ComponentType>(false);

        for (const auto &[propertyName, setter] : _registry.setters)
        {
            if (componentNode[propertyName])
            {
                YAML::Node propertyNode = componentNode[propertyName];
                setter(propertyNode, &script);
            }
        }
        if (_callCreate)
            script.Create();
    }
}

#define DEFAULT_DECODE(config, type) \
    Decode = [](YAML::Node &_node, Entity &_entity, bool _callCreate) -> void { DecodeComponent<type>(config.registry, _node, _entity, _callCreate); } \

/*#define CHECK_SCRIPTABLE_ENTITY(type) \
    Canis::Entity e; \
    type v(e); \
    if ((void*)dynamic_cast<Canis::ScriptableEntity*>(&v) == nullptr) { \
        Debug::Error("%s does NOT publicly inherit ScriptableEntity!", type_name<type>().data()); \
    } else { \
        Debug::Log("I Checked!"); \
    }*/
