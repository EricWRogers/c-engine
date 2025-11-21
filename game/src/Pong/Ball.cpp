#include "../../include/Pong/Ball.hpp"

#include <Canis/App.hpp>
#include <Canis/Time.hpp>
#include <Canis/Window.hpp>
#include <Canis/InputManager.hpp>

#include <map>

using namespace Canis;

namespace Pong
{

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
    Decode = [](YAML::Node &_node, Entity &_entity, bool _callCreate) -> void { DecodeComponent<type>(_node, _entity, _callCreate); } \

ScriptConf conf = {};

void RegisterBallScript(Canis::App& _app)
{
    REGISTER_PROPERTY(Pong::Ball, direction, Vector2);
    REGISTER_PROPERTY(Pong::Ball, speed, float);
    REGISTER_PROPERTY(Pong::Ball, randomRotation, float);

    conf.DEFAULT_NAME(Pong::Ball);
    conf.DEFAULT_ADD_AND_REQUIRED(Pong::Ball, Canis::RectTransform, Canis::Sprite2D);
    conf.DEFAULT_HAS(Pong::Ball);
    conf.DEFAULT_REMOVE(Pong::Ball);
    conf.DEFAULT_ENCODE(Pong::Ball);
    conf.DEFAULT_DECODE(Pong::Ball);

    conf.DrawInspector = [](Editor &_editor, Entity &_entity, const ScriptConf &_conf) -> void
    {
        Ball *ball = nullptr;
        if ((ball = _entity.GetScript<Ball>()) != nullptr)
        {
            ImGui::InputFloat2(("direction##" + _conf.name).c_str(), &ball->direction.x, "%.3f");
            ImGui::InputFloat(("speed##" + _conf.name).c_str(), &ball->speed);
            ImGui::InputFloat(("randomRotation##" + _conf.name).c_str(), &ball->randomRotation);
        }
    };
    
    _app.RegisterScript(conf);
}

DEFAULT_UNREGISTER_SCRIPT(Ball)

void Ball::Create()
{
    direction = Vector2(-1.0f, -1.0f).Normalize();
    direction = Vector2::Normalize(direction);
}

void Ball::Ready()
{
    direction = Vector2(-1.0f, 0.0f).Normalize();
    direction = Vector2::Normalize(direction);
}

void Ball::Destroy() {}

void Ball::Update(float _dt)
{
    Vector2 delta = direction * speed * Time::DeltaTime();
    transform.position += delta;
    transform.rotation += DEG2RAD * 5000.0f * Time::DeltaTime();

    CheckWalls();
}

void Ball::EditorInspectorDraw()
{
    std::string nameOfType = "Ball";
    ImGui::Text("%s", nameOfType.c_str());
    ImGui::InputFloat2("direction", &direction.x, "%.3f");
    ImGui::InputFloat("speed", &speed);
    ImGui::InputFloat("randomRotation", &randomRotation);
}

void Ball::CheckWalls()
{
    Canis::Window &window = entity.scene->GetWindow();

    if (window.GetScreenWidth() * 0.5f <= transform.position.x + transform.size.x * 0.5f)
    {
        if (direction.x > 0.0f)
        {
            direction.x *= -1.0f;
        }
    }
    else if (-window.GetScreenWidth() * 0.5f >= transform.position.x - transform.size.x * 0.5f)
    {
        if (direction.x < 0.0f)
        {
            direction.x *= -1.0f;
        }
    }
    else if (window.GetScreenHeight() * 0.5f <= transform.position.y + transform.size.y * 0.5f)
    {
        if (direction.y > 0.0f)
        {
            direction.y *= -1.0f;
        }
    }
    else if (-window.GetScreenHeight() * 0.5f >= transform.position.y - transform.size.y * 0.5f)
    {
        if (direction.y < 0.0f)
        {
            direction.y *= -1.0f;
        }
    }
}

} // namespace Pong