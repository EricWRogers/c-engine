#include <SpaceInvaders/Invader.hpp>

#include <Canis/App.hpp>
#include <Canis/ConfigHelper.hpp>

#include <cmath>

using namespace Canis;

namespace SpaceInvaders
{
    namespace
    {
        ScriptConf conf = {};
    }

    void RegisterInvaderScript(Canis::App &_app)
    {
        REGISTER_PROPERTY(conf, SpaceInvaders::Invader, points, int);
        REGISTER_PROPERTY(conf, SpaceInvaders::Invader, wobbleAmplitude, float);
        REGISTER_PROPERTY(conf, SpaceInvaders::Invader, wobbleSpeed, float);

        DEFAULT_CONFIG_AND_REQUIRED(conf, SpaceInvaders::Invader, Canis::RectTransform, Canis::Sprite2D);

        conf.DrawInspector = [](Editor &, Entity &_entity, const ScriptConf &_conf) -> void
        {
            Invader* invader = _entity.HasScript(SpaceInvaders::Invader::ScriptName)
                ? &_entity.GetScript<SpaceInvaders::Invader>()
                : nullptr;
            if (invader != nullptr)
            {
                ImGui::InputInt(("points##" + _conf.name).c_str(), &invader->points);
                ImGui::InputFloat(("wobbleAmplitude##" + _conf.name).c_str(), &invader->wobbleAmplitude);
                ImGui::InputFloat(("wobbleSpeed##" + _conf.name).c_str(), &invader->wobbleSpeed);
            }
        };

        _app.RegisterScript(conf);
    }

    DEFAULT_UNREGISTER_SCRIPT(conf, Invader)

    void Invader::Create() {}

    void Invader::Ready()
    {
        m_transform = entity.HasComponent<RectTransform>() ? &entity.GetComponent<RectTransform>() : nullptr;
    }

    void Invader::Destroy() {}

    void Invader::Update(float _dt)
    {
        m_transform = entity.HasComponent<RectTransform>() ? &entity.GetComponent<RectTransform>() : nullptr;
        if (m_transform == nullptr)
            return;

        m_time += _dt;
        m_transform->rotation = std::sin(m_time * wobbleSpeed) * wobbleAmplitude;
    }

    void Invader::EditorInspectorDraw() {}
}
