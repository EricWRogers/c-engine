#include <Canis/App.hpp>
#include <Canis/Entity.hpp>
#include <Canis/Scene.hpp>
#include <Canis/Window.hpp>
#include <Canis/AssetManager.hpp>
#include <Canis/Debug.hpp>


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

    m_scriptComponents.push_back(_scriptableEntity);

    if (_callCreate)
        _scriptableEntity->Create();

    return _scriptableEntity;
}

ScriptableEntity* Entity::GetScriptDirect(const ScriptConf& _conf)
{
    if (_conf.Get == nullptr)
        return nullptr;

    return static_cast<ScriptableEntity*>(_conf.Get(*this));
}

const ScriptableEntity* Entity::GetScriptDirect(const ScriptConf& _conf) const
{
    if (_conf.Get == nullptr)
        return nullptr;

    return static_cast<const ScriptableEntity*>(_conf.Get(const_cast<Entity&>(*this)));
}

void Entity::RemoveScriptDirect(const ScriptConf& _conf)
{
    ScriptableEntity* script = GetScriptDirect(_conf);
    if (script == nullptr)
        return;

    for (size_t i = 0; i < m_scriptComponents.size(); ++i)
    {
        if (m_scriptComponents[i] != script)
            continue;

        script->Destroy();
        delete script;
        m_scriptComponents.erase(m_scriptComponents.begin() + i);
        break;
    }
}

ScriptableEntity* Entity::AttachScript(const std::string& _scriptName, ScriptableEntity* _scriptableEntity, bool _callCreate)
{
    if (scene.app == nullptr)
    {
        delete _scriptableEntity;
        return nullptr;
    }

    ScriptConf* conf = scene.app->GetScriptConf(_scriptName);
    if (conf == nullptr)
    {
        delete _scriptableEntity;
        return nullptr;
    }

    return AddScriptDirect(*conf, _scriptableEntity, _callCreate);
}

void Entity::RemoveScript(const std::string& _scriptName)
{
    if (scene.app == nullptr)
        return;

    ScriptConf* conf = scene.app->GetScriptConf(_scriptName);
    if (conf == nullptr)
        return;

    RemoveScriptDirect(*conf);
}

void Entity::RemoveAllScripts()
{
    for (int i = static_cast<int>(m_scriptComponents.size()) - 1; i >= 0; --i)
    {
        ScriptableEntity* script = m_scriptComponents[static_cast<size_t>(i)];

        if (script != nullptr)
        {
            script->Destroy();
            delete script;
        }
    }

    m_scriptComponents.clear();
}

void Entity::Destroy() {
    scene.Destroy(id);
}

void Camera2D::Create() {
    if (entity == nullptr)
        return;

    m_screenWidth = entity->scene.GetWindow().GetScreenWidth();
    m_screenHeight = entity->scene.GetWindow().GetScreenHeight();
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

void Camera2D::UpdateMatrix()
{
    if (entity == nullptr)
        return;

    m_screenWidth = entity->scene.GetWindow().GetScreenWidth();
    m_screenHeight = entity->scene.GetWindow().GetScreenHeight();
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
