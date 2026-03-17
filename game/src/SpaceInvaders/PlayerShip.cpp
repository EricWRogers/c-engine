#include <SpaceInvaders/PlayerShip.hpp>

#include <SpaceInvaders/Projectile.hpp>
#include <SpaceInvaders/GameController.hpp>

#include <Canis/App.hpp>
#include <Canis/Scene.hpp>
#include <Canis/Time.hpp>
#include <Canis/InputManager.hpp>
#include <Canis/Window.hpp>
#include <Canis/AssetManager.hpp>
#include <Canis/ConfigHelper.hpp>

#include <algorithm>

using namespace Canis;

namespace SpaceInvaders
{
    namespace
    {
        ScriptConf conf = {};
    }

    void RegisterPlayerShipScript(Canis::App &_app)
    {
        REGISTER_PROPERTY(conf, SpaceInvaders::PlayerShip, speed, float);
        REGISTER_PROPERTY(conf, SpaceInvaders::PlayerShip, fireCooldown, float);
        REGISTER_PROPERTY(conf, SpaceInvaders::PlayerShip, bulletSpeed, float);
        REGISTER_PROPERTY(conf, SpaceInvaders::PlayerShip, bulletLifetime, float);

        DEFAULT_CONFIG_AND_REQUIRED(conf, SpaceInvaders::PlayerShip, Canis::RectTransform, Canis::Sprite2D);

        conf.DrawInspector = [](Editor &, Entity &_entity, const ScriptConf &_conf) -> void
        {
            if (_entity.HasScript<SpaceInvaders::PlayerShip>())
            {
                PlayerShip& ship = *_entity.GetScript<SpaceInvaders::PlayerShip>();
                ImGui::InputFloat(("speed##" + _conf.name).c_str(), &ship.speed);
                ImGui::InputFloat(("fireCooldown##" + _conf.name).c_str(), &ship.fireCooldown);
                ImGui::InputFloat(("bulletSpeed##" + _conf.name).c_str(), &ship.bulletSpeed);
                ImGui::InputFloat(("bulletLifetime##" + _conf.name).c_str(), &ship.bulletLifetime);
            }
        };

        _app.RegisterScript(conf);
    }

    DEFAULT_UNREGISTER_SCRIPT(conf, PlayerShip)

    void PlayerShip::Create() {}

    void PlayerShip::Ready()
    {
        m_transform = entity.GetComponent<RectTransform>();
        m_fireTimer = 0.0f;
    }

    void PlayerShip::Destroy() {}

    void PlayerShip::Update(float _dt)
    {
        m_transform = entity.GetComponent<RectTransform>();
        if (m_transform == nullptr)
            return;

        if (Entity* controllerEntity = entity.scene->FindEntityWithName("GameController"))
        {
            if (controllerEntity->HasScript<SpaceInvaders::GameController>())
            {
                GameController& controller = *controllerEntity->GetScript<SpaceInvaders::GameController>();
                if (controller.gameOver || controller.levelCleared)
                    return;
            }
        }

        InputManager& input = entity.scene->GetInputManager();

        float moveAxis = 0.0f;
        if (input.GetKey(Canis::Key::A) || input.GetKey(Canis::Key::LEFT))
            moveAxis -= 1.0f;
        if (input.GetKey(Canis::Key::D) || input.GetKey(Canis::Key::RIGHT))
            moveAxis += 1.0f;

        m_transform->position.x += moveAxis * speed * _dt;

        const float halfW = entity.scene->GetWindow().GetScreenWidth() * 0.5f;
        const float halfSize = m_transform->size.x * m_transform->GetScale().x * 0.5f;
        m_transform->position.x = std::clamp(m_transform->position.x, -halfW + halfSize + 8.0f, halfW - halfSize - 8.0f);

        m_fireTimer -= _dt;
        if (input.JustPressedKey(Canis::Key::SPACE) && m_fireTimer <= 0.0f)
        {
            m_fireTimer = fireCooldown;

            Entity* bulletEntity = entity.scene->CreateEntity("PlayerBullet", "PlayerBullet");
            RectTransform& bulletTransform = *bulletEntity->AddComponent<RectTransform>();
            Sprite2D& bulletSprite = *bulletEntity->AddComponent<Sprite2D>();
            Projectile* bullet = bulletEntity->AddScript<SpaceInvaders::Projectile>();

            bulletTransform.size = Vector2(14.0f, 24.0f);
            bulletTransform.position = m_transform->GetPosition() + Vector2(0.0f, m_transform->size.y * 0.55f);

            bulletSprite.textureHandle = AssetManager::GetTextureHandle("assets/textures/awesome_face.png");
            bulletSprite.color = Color(0.55f, 1.0f, 1.0f, 1.0f);

            bullet->fromPlayer = true;
            bullet->velocity = Vector2(0.0f, bulletSpeed);
            bullet->lifeTime = bulletLifetime;
        }
    }

    void PlayerShip::EditorInspectorDraw() {}

    void PlayerShip::OnHit()
    {
        if (Entity* controllerEntity = entity.scene->FindEntityWithName("GameController"))
        {
            if (controllerEntity->HasScript<SpaceInvaders::GameController>())
            {
                GameController& controller = *controllerEntity->GetScript<SpaceInvaders::GameController>();
                controller.OnPlayerHit();
                if (controller.gameOver)
                    return;
            }
        }

        m_transform = entity.GetComponent<RectTransform>();

        if (m_transform != nullptr)
            m_transform->position.x = 0.0f;
    }
}
