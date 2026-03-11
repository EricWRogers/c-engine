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

namespace
{
    bool MatchesScriptEntry(const ScriptComponentEntry& _entry, const ScriptConf& _conf)
    {
        return _entry.name == _conf.name;
    }
}

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

    m_scriptComponents.push_back(ScriptComponentEntry{
        .name = _conf.name,
        .script = _scriptableEntity
    });

    if (_callCreate)
        _scriptableEntity->Create();

    return _scriptableEntity;
}

ScriptableEntity* Entity::GetScriptDirect(const ScriptConf& _conf)
{
    for (ScriptComponentEntry& entry : m_scriptComponents)
    {
        if (MatchesScriptEntry(entry, _conf))
            return entry.script;
    }

    return nullptr;
}

const ScriptableEntity* Entity::GetScriptDirect(const ScriptConf& _conf) const
{
    for (const ScriptComponentEntry& entry : m_scriptComponents)
    {
        if (MatchesScriptEntry(entry, _conf))
            return entry.script;
    }

    return nullptr;
}

void Entity::RemoveScriptDirect(const ScriptConf& _conf)
{
    for (size_t i = 0; i < m_scriptComponents.size(); ++i)
    {
        if (!MatchesScriptEntry(m_scriptComponents[i], _conf))
            continue;

        ScriptableEntity* script = m_scriptComponents[i].script;
        if (script != nullptr)
        {
            script->Destroy();
            delete script;
        }

        m_scriptComponents.erase(m_scriptComponents.begin() + i);
        break;
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
    for (int i = static_cast<int>(m_scriptComponents.size()) - 1; i >= 0; --i)
    {
        ScriptableEntity* script = m_scriptComponents[static_cast<size_t>(i)].script;

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

void DirectionalLight::EditorInspectorDraw()
{
    std::string nameOfType = "DirectionalLight";
    ImGui::Text("%s", nameOfType.c_str());
    ImGui::Checkbox("enabled", &enabled);
    ImGui::ColorEdit3("color", &color.r);
    ImGui::InputFloat("intensity", &intensity);
    ImGui::InputFloat3("direction", &direction.x, "%.3f");
}

void PointLight::EditorInspectorDraw()
{
    std::string nameOfType = "PointLight";
    ImGui::Text("%s", nameOfType.c_str());
    ImGui::Checkbox("enabled", &enabled);
    ImGui::ColorEdit3("color", &color.r);
    ImGui::InputFloat("intensity", &intensity);
    ImGui::InputFloat("range", &range);
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

void Camera2D::Create() {
    if (entity == nullptr || entity->scene == nullptr)
        return;

    m_screenWidth = entity->scene->GetWindow().GetScreenWidth();
    m_screenHeight = entity->scene->GetWindow().GetScreenHeight();
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
    if (entity == nullptr || entity->scene == nullptr)
        return;

    m_screenWidth = entity->scene->GetWindow().GetScreenWidth();
    m_screenHeight = entity->scene->GetWindow().GetScreenHeight();
    m_projection = glm::ortho(0.0f, (float)m_screenWidth, 0.0f,
                                      (float)m_screenHeight, 0.0f, 100.0f);
                                      
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
