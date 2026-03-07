#include <Canis/App.hpp>
#include <Canis/Entity.hpp>
#include <Canis/Scene.hpp>
#include <Canis/Window.hpp>
#include <Canis/AssetManager.hpp>
#include <Canis/Debug.hpp>
#include <Canis/ConfigData.hpp>

#include <imgui.h>
#include <imgui_stdlib.h>

namespace Canis {

ScriptableEntity* Entity::AddScriptDirect(const ScriptConf& _conf, ScriptableEntity* _scriptableEntity, bool _callCreate)
{
    if (_scriptableEntity == nullptr)
        return nullptr;

    if (ScriptableEntity* existing = GetScriptDirect(_conf))
    {
        if (existing != _scriptableEntity)
            delete _scriptableEntity;
        return existing;
    }

    if (scene == nullptr)
    {
        delete _scriptableEntity;
        return nullptr;
    }

    ScriptableEntity* added = scene->AddComponentToEntity(
        static_cast<u32>(id),
        _conf.componentIndex,
        _conf.componentMask,
        _scriptableEntity
    );

    if (added == nullptr)
    {
        delete _scriptableEntity;
        return nullptr;
    }

    if (added != _scriptableEntity)
    {
        delete _scriptableEntity;
        return added;
    }

    m_scriptComponents.push_back(ScriptComponentEntry{
        .name = _conf.name,
        .componentIndex = _conf.componentIndex,
        .componentMask = _conf.componentMask,
        .script = _scriptableEntity
    });

    if (_callCreate)
        _scriptableEntity->Create();

    return _scriptableEntity;
}

ScriptableEntity* Entity::GetScriptDirect(const ScriptConf& _conf)
{
    if (scene == nullptr)
        return nullptr;

    return scene->GetComponentFromEntity(static_cast<u32>(id), _conf.componentIndex);
}

const ScriptableEntity* Entity::GetScriptDirect(const ScriptConf& _conf) const
{
    if (scene == nullptr)
        return nullptr;

    return scene->GetComponentFromEntity(static_cast<u32>(id), _conf.componentIndex);
}

void Entity::RemoveScriptDirect(const ScriptConf& _conf)
{
    if (scene == nullptr)
        return;

    ScriptableEntity* script = scene->RemoveComponentFromEntity(static_cast<u32>(id), _conf.componentIndex);
    if (script == nullptr)
        return;

    script->Destroy();
    delete script;

    for (size_t i = 0; i < m_scriptComponents.size(); ++i)
    {
        if (m_scriptComponents[i].componentIndex == _conf.componentIndex)
        {
            m_scriptComponents.erase(m_scriptComponents.begin() + i);
            break;
        }
    }
}

ScriptableEntity* Entity::AddScript(const std::string& _scriptName, bool _callCreate)
{
    if (scene == nullptr || scene->app == nullptr)
        return nullptr;

    ScriptConf* conf = scene->app->GetScriptConf(_scriptName);
    if (conf == nullptr)
        return nullptr;

    if (ScriptableEntity* existing = GetScriptDirect(*conf))
        return existing;

    if (!_callCreate && conf->Construct)
        return conf->Construct(*this, false);

    static thread_local std::vector<std::string> addStack = {};
    if (std::find(addStack.begin(), addStack.end(), _scriptName) != addStack.end())
    {
        if (conf->Construct)
            return conf->Construct(*this, _callCreate);

        return nullptr;
    }

    addStack.push_back(_scriptName);
    struct StackPopGuard
    {
        std::vector<std::string>& stack;
        ~StackPopGuard() { stack.pop_back(); }
    } guard{ addStack };

    if (conf->Add)
        conf->Add(*this);
    else if (conf->Construct)
        conf->Construct(*this, _callCreate);

    return GetScriptDirect(*conf);
}

ScriptableEntity* Entity::AttachScript(const std::string& _scriptName, ScriptableEntity* _scriptableEntity, bool _callCreate)
{
    if (scene == nullptr || scene->app == nullptr)
    {
        delete _scriptableEntity;
        return nullptr;
    }

    ScriptConf* conf = scene->app->GetScriptConf(_scriptName);
    if (conf == nullptr)
    {
        delete _scriptableEntity;
        return nullptr;
    }

    return AddScriptDirect(*conf, _scriptableEntity, _callCreate);
}

ScriptableEntity* Entity::GetScript(const std::string& _scriptName)
{
    if (scene == nullptr || scene->app == nullptr)
        return nullptr;

    ScriptConf* conf = scene->app->GetScriptConf(_scriptName);
    if (conf == nullptr)
        return nullptr;

    return GetScriptDirect(*conf);
}

const ScriptableEntity* Entity::GetScript(const std::string& _scriptName) const
{
    if (scene == nullptr || scene->app == nullptr)
        return nullptr;

    ScriptConf* conf = scene->app->GetScriptConf(_scriptName);
    if (conf == nullptr)
        return nullptr;

    return GetScriptDirect(*conf);
}

void Entity::RemoveScript(const std::string& _scriptName)
{
    if (scene == nullptr || scene->app == nullptr)
        return;

    ScriptConf* conf = scene->app->GetScriptConf(_scriptName);
    if (conf == nullptr)
        return;

    RemoveScriptDirect(*conf);
}

bool Entity::HasScript(const std::string& _scriptName) const
{
    return GetScript(_scriptName) != nullptr;
}

void Entity::RemoveAllScripts()
{
    if (scene == nullptr)
    {
        for (ScriptComponentEntry& entry : m_scriptComponents)
        {
            if (entry.script == nullptr)
                continue;

            entry.script->Destroy();
            delete entry.script;
        }

        m_scriptComponents.clear();
        return;
    }

    for (int i = static_cast<int>(m_scriptComponents.size()) - 1; i >= 0; --i)
    {
        ScriptComponentEntry entry = m_scriptComponents[static_cast<size_t>(i)];
        ScriptableEntity* script = scene->RemoveComponentFromEntity(static_cast<u32>(id), entry.componentIndex);
        if (script == nullptr)
            script = entry.script;

        if (script != nullptr)
        {
            script->Destroy();
            delete script;
        }
    }

    m_scriptComponents.clear();
}

void Entity::Destroy() {
    scene->Destroy(id);
}

void RectTransform::EditorInspectorDraw()
{
    std::string nameOfType = "RectTransform";
    ImGui::Text("%s", nameOfType.c_str());
    ImGui::InputFloat2("position", &position.x, "%.3f");
    ImGui::InputFloat2("size", &size.x, "%.3f");
    ImGui::InputFloat2("scale", &scale.x);
    ImGui::InputFloat2("originOffset", &originOffset.x, "%.3f");
    ImGui::InputFloat("depth", &depth);
    // let user work with degrees
    float degrees = RAD2DEG * rotation;
    ImGui::InputFloat("rotation", &degrees);
    rotation = DEG2RAD * degrees;
}

void Sprite2D::EditorInspectorDraw()
{
    std::string nameOfType = "Sprite2D";
    ImGui::Text("%s", nameOfType.c_str());
    ImGui::ColorEdit4("color", &color.r);
    ImGui::InputFloat4("uv", &uv.x, "%.3f");
}

void Text::EditorInspectorDraw()
{
    std::string nameOfType = "Text";
    ImGui::Text("%s", nameOfType.c_str());
    ImGui::InputText("text", &text);
    ImGui::InputInt("font asset id", &assetId);
}

void Transform3D::EditorInspectorDraw()
{
    std::string nameOfType = "Transform3D";
    ImGui::Text("%s", nameOfType.c_str());
    ImGui::InputFloat3("position", &position.x, "%.3f");

    Vector3 degrees = rotation * RAD2DEG;
    ImGui::InputFloat3("rotation", &degrees.x, "%.3f");
    rotation = degrees * DEG2RAD;

    ImGui::InputFloat3("scale", &scale.x, "%.3f");
}

void Camera3D::EditorInspectorDraw()
{
    std::string nameOfType = "Camera3D";
    ImGui::Text("%s", nameOfType.c_str());
    ImGui::Checkbox("primary", &primary);
    ImGui::InputFloat("fovDegrees", &fovDegrees);
    ImGui::InputFloat("nearClip", &nearClip);
    ImGui::InputFloat("farClip", &farClip);
}

void Model3D::EditorInspectorDraw()
{
    std::string nameOfType = "Model3D";
    ImGui::Text("%s", nameOfType.c_str());
    ImGui::InputInt("modelId", &modelId);
    ImGui::ColorEdit4("color", &color.r);
}

void Material::EditorInspectorDraw()
{
    std::string nameOfType = "Material";
    ImGui::Text("%s", nameOfType.c_str());
    ImGui::InputInt("materialId", &materialId);
    ImGui::ColorEdit4("color", &color.r);
}

void ModelAnimation3D::EditorInspectorDraw()
{
    std::string nameOfType = "ModelAnimation3D";
    ImGui::Text("%s", nameOfType.c_str());
    ImGui::Checkbox("playAnimation", &playAnimation);
    ImGui::Checkbox("loop", &loop);
    ImGui::InputFloat("animationSpeed", &animationSpeed);
    ImGui::InputFloat("animationTime", &animationTime);
    ImGui::InputInt("animationIndex", &animationIndex);
}

Camera2D::Camera2D(Canis::Entity &_entity)
    : Canis::ScriptableEntity(_entity), m_position(0.0f, 0.0f), m_scale(8.0f),
      m_needsMatrixUpdate(true), m_screenWidth(500), m_screenHeight(500) {
    m_cameraMatrix = Matrix4(1.0f);
    m_view = Matrix4(1.0f);
    m_projection = Matrix4(1.0f);
}

Camera2D::~Camera2D() {}

void Camera2D::Create() {
    m_screenWidth = entity.scene->GetWindow().GetScreenWidth();
    m_screenHeight = entity.scene->GetWindow().GetScreenHeight();
    m_projection = glm::ortho(0.0f, (float)m_screenWidth, 0.0f,
                                      (float)m_screenHeight, 0.0f, 100.0f);
    SetPosition(Vector2(0.0f)); // Vector2((float)m_screenWidth / 2,
                                // (float)m_screenHeight / 2));
    SetScale(1.0f);
}

void Camera2D::Destroy() {
    Debug::Log("DestroyCamera");
}

void Camera2D::Update(float _dt) {}

void Camera2D::EditorInspectorDraw()
{
    std::string nameOfType = "Camera2D";
    ImGui::Text("%s", nameOfType.c_str());

    Vector2 lastPosition = GetPosition();
    float lastScale = GetScale();

    ImGui::InputFloat2("position", &lastPosition.x, "%.3f");
    ImGui::InputFloat("scale", &lastScale);

    if (lastPosition != GetPosition())
        SetPosition(lastPosition);

    if (lastScale != GetScale())
        SetScale(lastScale);
}

void Camera2D::UpdateMatrix()
{
    m_view = Matrix4(1.0f);
    m_view = glm::translate(m_view, Vector3(-m_position.x + m_screenWidth / 2,
                                             -m_position.y + m_screenHeight / 2, 0.0f));
    m_view = glm::scale(m_view, Vector3(m_scale, m_scale, 0.0f));

    m_cameraMatrix = m_projection * m_view;

    m_needsMatrixUpdate = false;
}

void SpriteAnimation::Play(std::string _path)
{
    speed = 1.0f;
    id = AssetManager::LoadSpriteAnimation(_path);
    index = 0;
    redraw = true;
}
} // namespace Canis
