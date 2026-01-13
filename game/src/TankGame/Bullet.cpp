#include <TankGame/Bullet.hpp>

#include <Canis/App.hpp>
#include <Canis/Time.hpp>
#include <Canis/Math.hpp>
#include <Canis/Scene.hpp>
#include <Canis/Window.hpp>
#include <Canis/Editor.hpp>
#include <Canis/InputManager.hpp>

#include <ConfigHelper.hpp>

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
            TankGame::Bullet *bullet = nullptr;
            if ((bullet = _entity.GetScript<TankGame::Bullet>()) != nullptr)
            {
                ImGui::InputFloat(("speed##" + _conf.name).c_str(), &bullet->speed);
                ImGui::InputFloat(("lifeTime##" + _conf.name).c_str(), &bullet->lifeTime);
            }
        };

        _app.RegisterScript(conf);
    }

    DEFAULT_UNREGISTER_SCRIPT(conf, Bullet)

    void Bullet::Create() { Debug::Log("Tank But No Bullet!"); }

    void Bullet::Ready() {
        m_transform = entity.GetScript<Canis::RectTransform>();
    }

    void Bullet::Destroy() { /*Debug::Log("Kill Bullet But No Bullet!");*/ }

    void Bullet::Update(float _dt) {
        m_transform->Move(m_transform->GetUp() * speed * _dt);

        lifeTime -= _dt;

        //Debug::Log("LifeTime: %f", lifeTime);

        if (lifeTime <= 0.0f) {
            entity.Destroy();
        }
        
    }

    void Bullet::EditorInspectorDraw() {}
}
