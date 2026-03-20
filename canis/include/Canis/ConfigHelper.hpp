#pragma once
#include <Canis/ConfigData.hpp>
#include <Canis/Entity.hpp>
#include <Canis/Scene.hpp>
#include <Canis/Editor.hpp>
#include <Canis/Yaml.hpp>

#include <algorithm>
#include <cstdio>
#include <type_traits>
#include <utility>

#include <imgui.h>
#include <imgui_stdlib.h>

using namespace Canis;

#define DEFAULT_NAME(type) \
    name = type::ScriptName \

#define DEFAULT_ADD(type) \
    Add = [](Entity &_entity) -> void { (void)_entity.AddScript<type>(); } \

#define DEFAULT_CONSTRUCT(type) \
    Construct = [](Entity &_entity, bool _callCreate) -> ScriptableEntity* { return _entity.AttachScript(type::ScriptName, new type(_entity), _callCreate); } \

template<typename... Ts>
inline void AddRequiredScripts(Entity& _entity)
{
    (_entity.scene.app->AddRequiredScript(_entity, Ts::ScriptName), ...);
}

template <typename T>
inline void DrawInspectorField(const char *_label, T &_value)
{
    if constexpr (std::is_same_v<T, bool>)
    {
        ImGui::Checkbox(_label, &_value);
    }
    else if constexpr (std::is_same_v<T, int>)
    {
        ImGui::InputInt(_label, &_value);
    }
    else if constexpr (std::is_same_v<T, float>)
    {
        ImGui::InputFloat(_label, &_value);
    }
    else if constexpr (std::is_same_v<T, double>)
    {
        ImGui::InputScalar(_label, ImGuiDataType_Double, &_value);
    }
    else if constexpr (std::is_same_v<T, Vector2>)
    {
        ImGui::InputFloat2(_label, &_value.x);
    }
    else if constexpr (std::is_same_v<T, Vector3>)
    {
        ImGui::InputFloat3(_label, &_value.x);
    }
    else if constexpr (std::is_same_v<T, Vector4>)
    {
        ImGui::InputFloat4(_label, &_value.x);
    }
    else if constexpr (std::is_same_v<T, std::string>)
    {
        ImGui::InputText(_label, &_value);
    }
    else if constexpr (std::is_same_v<T, Mask>)
    {
        ImGui::PushID(_label);

        const char *visibleLabelEnd = _label;
        while (*visibleLabelEnd != '\0')
        {
            if (visibleLabelEnd[0] == '#' && visibleLabelEnd[1] == '#')
                break;

            ++visibleLabelEnd;
        }

        if (visibleLabelEnd != _label)
        {
            ImGui::TextUnformatted(_label, visibleLabelEnd);
            ImGui::SameLine();
            ImGui::TextDisabled("0x%08X", static_cast<u32>(_value));
        }

        const ImVec4 activeColor = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
        const ImVec4 hoveredColor = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
        const float buttonWidth = ImGui::CalcTextSize("32").x + (ImGui::GetStyle().FramePadding.x * 2.0f);
        const float groupSpacing = ImGui::GetStyle().ItemSpacing.x * 2.0f;

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f, 2.0f));

        for (int group = 0; group < 4; ++group)
        {
            if (group > 0)
                ImGui::SameLine(0.0f, groupSpacing);

            ImGui::BeginGroup();
            for (int row = 0; row < 2; ++row)
            {
                for (int col = 0; col < 4; ++col)
                {
                    if (col > 0)
                        ImGui::SameLine();

                    const int bitIndex = (group * 8) + (row * 4) + col;
                    const bool enabled = _value.HasBit(bitIndex);

                    if (enabled)
                    {
                        ImGui::PushStyleColor(ImGuiCol_Button, activeColor);
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoveredColor);
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, activeColor);
                    }

                    char bitLabel[8];
                    std::snprintf(bitLabel, sizeof(bitLabel), "%d", bitIndex + 1);

                    if (ImGui::Button(bitLabel, ImVec2(buttonWidth, 0.0f)))
                        _value.ToggleBit(bitIndex);

                    if (enabled)
                        ImGui::PopStyleColor(3);

                    if (ImGui::IsItemHovered())
                    {
                        ImGui::BeginTooltip();
                        ImGui::Text("Bit %d, value %u", bitIndex + 1, u32(1u) << bitIndex);
                        ImGui::EndTooltip();
                    }
                }
            }
            ImGui::EndGroup();
        }

        ImGui::PopStyleVar();
        ImGui::PopID();
    }
    else if constexpr (std::is_integral_v<T>)
    {
        long long value = static_cast<long long>(_value);
        if (ImGui::InputScalar(_label, ImGuiDataType_S64, &value))
            _value = static_cast<T>(value);
    }
    else if constexpr (std::is_floating_point_v<T>)
    {
        double value = static_cast<double>(_value);
        if (ImGui::InputScalar(_label, ImGuiDataType_Double, &value))
            _value = static_cast<T>(value);
    }
    else
    {
        ImGui::TextDisabled("%s (unsupported type)", _label);
    }
}

inline void DrawRegisteredProperties(const PropertyRegistry &_registry, void *_component, const std::string &_idSuffix)
{
    for (const std::string &propertyName : _registry.propertyOrder)
    {
        auto drawerIt = _registry.drawers.find(propertyName);
        if (drawerIt == _registry.drawers.end())
            continue;

        drawerIt->second(propertyName, _component, _idSuffix);
    }
}

#define DEFAULT_ADD_AND_REQUIRED(type, ...)                               \
    Add = [](Entity &_entity)   -> void                                   \
    {                                                                     \
        AddRequiredScripts<__VA_ARGS__>(_entity);                         \
        (void)_entity.AddScript<type>();                                  \
    }                                                                     \

#define DEFAULT_HAS(type) \
    Has = [](Entity &_entity) -> bool { return _entity.HasScript<type>(); } \

#define DEFAULT_REMOVE(type) \
    Remove = [](Entity &_entity) -> void { _entity.RemoveScript<type>(); } \

#define DEFAULT_GET(type) \
    Get = [](Entity& _entity) -> void* { return (void*)_entity.GetScript<type>(); } \

#define DEFAULT_DRAW_INSPECTOR(type, ...)                                             \
    DrawInspector = [](Editor &_editor, Entity &_entity, const ScriptConf &_conf) -> void \
    {                                                                                 \
        (void)_editor;                                                                \
        if (type *component = _entity.GetScript<type>())                              \
        {                                                                             \
            DrawRegisteredProperties(_conf.registry, component, _conf.name);          \
            __VA_ARGS__                                                               \
        }                                                                             \
    }                                                                                 \

#define DECODE(node, component, property) \
    component->property = node[#property].as<decltype(component->property)>(component->property); \

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

#define REGISTER_PROPERTY(config, component, property)                                            \
{                                                                                                   \
    using PropertyType = std::remove_cvref_t<decltype(std::declval<component>().property)>;       \
    config.registry.setters[#property] = [](YAML::Node &node, void *componentPtr) {             	        \
        static_cast<component *>(componentPtr)->property = node.as<PropertyType>();                 \
    };                                                                                              \
                                                                                                    \
    config.registry.getters[#property] = [](void *componentPtr) -> YAML::Node {                     		\
        return YAML::Node(static_cast<component *>(componentPtr)->property);                        \
    };                                                                                         		\
                                                                                                    \
    config.registry.drawers[#property] = [](const std::string &propertyName, void *componentPtr, const std::string &idSuffix) { \
        auto *typedComponent = static_cast<component *>(componentPtr);                              \
        DrawInspectorField<PropertyType>((propertyName + "##" + idSuffix).c_str(), typedComponent->property); \
    };                                                                                               \
                                                                                               		\
    config.registry.propertyOrder.push_back(#property);                                             		\
}

#define UNREGISTER_PROPERTY(registry, component, property)                          \
{                                                                                   \
    registry.setters.erase(#property);                                              \
    registry.getters.erase(#property);                                              \
    registry.drawers.erase(#property);                                              \
                                                                                    \
    auto &order = registry.propertyOrder;                                           \
    order.erase(std::remove(order.begin(), order.end(), std::string(#property)),    \
                order.end());                                                       \
}                                                                                   \

template <typename T>
inline void EncodeComponent(PropertyRegistry& _registry, YAML::Node &_node, Entity &_entity, const std::string& _scriptName)
{
    if (ScriptableEntity* component = static_cast<ScriptableEntity*>(_entity.GetScript<T>()))
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
    Encode = [](YAML::Node &_node, Entity &_entity) -> void { EncodeComponent<type>(config.registry, _node, _entity, type::ScriptName); } \

template <typename T>    
inline void DecodeComponent(PropertyRegistry& _registry, YAML::Node &_node, Canis::Entity &_entity, const std::string& _scriptName, bool _callCreate)
{
    if (auto componentNode = _node[_scriptName])
    {
        ScriptableEntity* script = static_cast<ScriptableEntity*>(_entity.AddScript<T>(false));
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
    Decode = [](YAML::Node &_node, Entity &_entity, bool _callCreate) -> void { DecodeComponent<type>(config.registry, _node, _entity, type::ScriptName, _callCreate); } \

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
        (void)_entity.AddScript<type>();                \
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
