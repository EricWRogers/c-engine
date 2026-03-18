#include <TankGame/FollowMouse.hpp>

#include <Canis/App.hpp>
#include <Canis/Scene.hpp>
#include <Canis/Window.hpp>
#include <Canis/Editor.hpp>
#include <Canis/InputManager.hpp>

#include <Canis/ConfigHelper.hpp>

using namespace Canis;

namespace TankGame
{
    ScriptConf followMouseConf = {};

    void RegisterFollowMouseScript(Canis::App &_app)
    {
        REGISTER_PROPERTY(followMouseConf, TankGame::FollowMouse, offset);

        DEFAULT_CONFIG_AND_REQUIRED(followMouseConf, TankGame::FollowMouse, Canis::RectTransform);

        followMouseConf.DrawInspector = [](Editor &_editor, Entity &_entity, const ScriptConf &_conf) -> void
        {
            TankGame::FollowMouse* followMouse = _entity.GetScript<TankGame::FollowMouse>();
            if (followMouse != nullptr)
            {
                ImGui::InputFloat2(("offset##" + _conf.name).c_str(), &followMouse->offset.x, "%.3f");
            }
        };

        _app.RegisterScript(followMouseConf);
    }

    DEFAULT_UNREGISTER_SCRIPT(followMouseConf, FollowMouse)

    void FollowMouse::Create() {}

    void FollowMouse::Ready()
    {
        m_transform = entity.HasComponent<RectTransform>() ? &entity.GetComponent<RectTransform>() : nullptr;
    }

    void FollowMouse::Destroy() {}

    void FollowMouse::Update(float _dt)
    {
        (void)_dt;

        if (m_transform == nullptr || entity.scene == nullptr)
            return;

        Window& window = entity.scene->GetWindow();
        InputManager& input = entity.scene->GetInputManager();

        Vector2 target = input.mouse - Vector2(
            static_cast<float>(window.GetScreenWidth()),
            static_cast<float>(window.GetScreenHeight())) * 0.5f;

        Camera2D* camera2D = nullptr;
        for (Entity* e : entity.scene->GetEntities())
        {
            if (e == nullptr || !e->active)
                continue;

            Camera2D* camera = e->HasComponent<Camera2D>() ? &e->GetComponent<Camera2D>() : nullptr;
            if (camera != nullptr)
            {
                camera2D = camera;
                break;
            }
        }

        if (camera2D != nullptr)
        {
            const float camScale = camera2D->GetScale();
            if (camScale != 0.0f)
                target /= camScale;

            target += camera2D->GetPosition();
        }

        m_transform->SetPosition(target + offset);
    }

    void FollowMouse::EditorInspectorDraw() {}
}
