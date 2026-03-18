#include <TankGame/Bounce.hpp>

#include <Canis/App.hpp>
#include <Canis/Scene.hpp>
#include <Canis/Editor.hpp>
#include <Canis/InputManager.hpp>
#include <Canis/Math.hpp>

#include <Canis/ConfigHelper.hpp>

using namespace Canis;

namespace TankGame
{
    ScriptConf bounceConf = {};

    void RegisterBounceScript(Canis::App &_app)
    {
        REGISTER_PROPERTY(bounceConf, TankGame::Bounce, scaleMultiplier);
        REGISTER_PROPERTY(bounceConf, TankGame::Bounce, duration);

        DEFAULT_CONFIG_AND_REQUIRED(bounceConf, TankGame::Bounce, Canis::RectTransform);

        bounceConf.DrawInspector = [](Editor &_editor, Entity &_entity, const ScriptConf &_conf) -> void
        {
            TankGame::Bounce* bounce = _entity.GetScript<TankGame::Bounce>();
            if (bounce != nullptr)
            {
                ImGui::InputFloat(("scaleMultiplier##" + _conf.name).c_str(), &bounce->scaleMultiplier);
                ImGui::InputFloat(("duration##" + _conf.name).c_str(), &bounce->duration);
            }
        };

        _app.RegisterScript(bounceConf);
    }

    DEFAULT_UNREGISTER_SCRIPT(bounceConf, Bounce)

    void Bounce::Create() {}

    void Bounce::Ready()
    {
        m_transform = entity.HasComponent<RectTransform>() ? &entity.GetComponent<RectTransform>() : nullptr;
        if (m_transform != nullptr)
            m_restScale = m_transform->scale;
    }

    void Bounce::Destroy() {}

    void Bounce::Update(float _dt)
    {
        if (m_transform == nullptr)
            return;

        InputManager& input = entity.scene.GetInputManager();

        if (input.JustLeftClicked())
        {
            m_restScale = m_transform->scale;
            m_elapsed = 0.0f;
            m_isPlaying = true;
        }

        if (!m_isPlaying)
            return;

        const float safeDuration = (duration <= 0.0001f) ? 0.0001f : duration;
        m_elapsed += _dt;
        const float t = m_elapsed / safeDuration;

        if (t >= 1.0f)
        {
            m_transform->scale = m_restScale;
            m_isPlaying = false;
            return;
        }

        const float safeMultiplier = (scaleMultiplier < 1.0f) ? 1.0f : scaleMultiplier;
        const float pulse = (t < 0.5f)
            ? Canis::Lerp(1.0f, safeMultiplier, t * 2.0f)
            : Canis::Lerp(safeMultiplier, 1.0f, (t - 0.5f) * 2.0f);

        m_transform->scale = m_restScale * pulse;
    }

    void Bounce::EditorInspectorDraw() {}
}
