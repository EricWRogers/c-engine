#include <TankGame/Bullet.hpp>

#include <Canis/App.hpp>
#include <Canis/Time.hpp>
#include <Canis/Math.hpp>
#include <Canis/Scene.hpp>
#include <Canis/Window.hpp>
#include <Canis/Editor.hpp>
#include <Canis/InputManager.hpp>

#include <Canis/ConfigHelper.hpp>

using namespace Canis;

namespace TankGame
{

    ScriptConf conf = {};

    void RegisterBulletScript(Canis::App &_app)
    {
        REGISTER_PROPERTY(conf, TankGame::Bullet, speed, float);
        REGISTER_PROPERTY(conf, TankGame::Bullet, lifeTime, float);

        DEFAULT_CONFIG_AND_REQUIRED(conf, TankGame::Bullet, Canis::RectTransform, Canis::Sprite2D);

        conf.DrawInspector = [](Editor &_editor, Entity &_entity, const ScriptConf &_conf) -> void
        {
            if (TankGame::Bullet *bullet = CANIS_GET_SCRIPT(_entity, TankGame::Bullet))
            {
                ImGui::InputFloat(("speed##" + _conf.name).c_str(), &bullet->speed);
                ImGui::InputFloat(("lifeTime##" + _conf.name).c_str(), &bullet->lifeTime);
            }
        };

        _app.RegisterScript(conf);
    }

    DEFAULT_UNREGISTER_SCRIPT(conf, Bullet)

    void Bullet::Create() { }

    void Bullet::Ready() {
        m_transform = CANIS_GET_COMPONENT(entity, RectTransform);
    }

    void Bullet::Destroy() { }

    void Bullet::Update(float _dt) {
        m_transform = CANIS_GET_COMPONENT(entity, RectTransform);
        if (m_transform == nullptr)
            return;

        m_transform->Move(m_transform->GetUp() * speed * _dt);

        lifeTime -= _dt;

        if (lifeTime <= 0.0f) {
            entity.Destroy();
        }
        
    }

    void Bullet::EditorInspectorDraw() {}
}
