#pragma once

#include <Canis/Entity.hpp>
#include <Canis/Scene.hpp>
#include <Canis/Editor.hpp>

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

#define DEFAULT_UNREGISTER_SCRIPT(type)         \
void UnRegister##type##Script(Canis::App& _app) \
{                                               \
    _app.UnregisterScript(conf);                \
}

using PropertySetter = std::function<void(YAML::Node&, void*)>;
using PropertyGetter = std::function<YAML::Node(void*)>;

struct PropertyRegistry {
    std::map<std::string, PropertySetter> setters;
    std::map<std::string, PropertyGetter> getters;
	std::vector<std::string> propertyOrder;
};

template <typename T>
PropertyRegistry& GetPropertyRegistry()
{
    static PropertyRegistry registry;
    return registry;
}

#define REGISTER_PROPERTY(component, property, type)                                                                 		\
{                                                                                                                    		\
    GetPropertyRegistry<component>().setters[#property] = [](YAML::Node &node, void *componentPtr) {             	        \
        static_cast<component *>(componentPtr)->property = node.as<type>();                                         		\
    };                                                                                                               		\
                                                                                                                     		\
    GetPropertyRegistry<component>().getters[#property] = [](void *componentPtr) -> YAML::Node {                     		\
        return YAML::Node(static_cast<component *>(componentPtr)->property);                                     		    \
    };                                                                                                               		\
                                                                                                                     		\
    GetPropertyRegistry<component>().propertyOrder.push_back(#property);                                             		\
}

template <typename ComponentType>
void EncodeComponent(YAML::Node &_node, Entity &_entity)
{
    if (_entity.GetScript<ComponentType>())
    {
        ComponentType &component = *_entity.GetScript<ComponentType>();

        YAML::Node comp;

        auto &registry = GetPropertyRegistry<ComponentType>();
        for (const auto &propertyName : registry.propertyOrder)
        {
            comp[propertyName] = registry.getters[propertyName](&component);
        }

        _node[type_name<ComponentType>()] = comp;
    }
}

#define DEFAULT_ENCODE(type) \
    Encode = [](YAML::Node &_node, Entity &_entity) -> void { EncodeComponent<type>(_node, _entity); } \

template <typename ComponentType>
void DecodeComponent(YAML::Node &_node, Canis::Entity &_entity, bool _callCreate)
{
    if (auto componentNode = _node[std::string(type_name<ComponentType>())])
    {
        auto &script = *_entity.AddScript<ComponentType>(false);
        auto &registry = GetPropertyRegistry<ComponentType>();

        for (const auto &[propertyName, setter] : registry.setters)
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

#define DEFAULT_DECODE(type) \
    Decode = [](YAML::Node &_node, Entity &_entity, bool _callCreate) -> void { DecodeComponent<type>(_node, _entity, _callCreate); }

/*#define CHECK_SCRIPTABLE_ENTITY(type) \
    Canis::Entity e; \
    type v(e); \
    if ((void*)dynamic_cast<Canis::ScriptableEntity*>(&v) == nullptr) { \
        Debug::Error("%s does NOT publicly inherit ScriptableEntity!", type_name<type>().data()); \
    } else { \
        Debug::Log("I Checked!"); \
    }*/
