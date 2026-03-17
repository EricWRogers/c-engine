#include <SpaceInvaders/SwarmController.hpp>

#include <SpaceInvaders/Projectile.hpp>
#include <SpaceInvaders/GameController.hpp>

#include <Canis/App.hpp>
#include <Canis/Scene.hpp>
#include <Canis/Window.hpp>
#include <Canis/AssetManager.hpp>
#include <Canis/ConfigHelper.hpp>

#include <random>

using namespace Canis;

namespace SpaceInvaders
{
    namespace
    {
        ScriptConf conf = {};
        std::mt19937 g_rng(std::random_device{}());
    }

    void RegisterSwarmControllerScript(Canis::App &_app)
    {
        REGISTER_PROPERTY(conf, SpaceInvaders::SwarmController, horizontalSpeed, float);
        REGISTER_PROPERTY(conf, SpaceInvaders::SwarmController, stepDown, float);
        REGISTER_PROPERTY(conf, SpaceInvaders::SwarmController, fireInterval, float);
        REGISTER_PROPERTY(conf, SpaceInvaders::SwarmController, enemyBulletSpeed, float);
        REGISTER_PROPERTY(conf, SpaceInvaders::SwarmController, enemyBulletLifetime, float);

        DEFAULT_CONFIG(conf, SpaceInvaders::SwarmController);

        conf.DrawInspector = [](Editor &, Entity &_entity, const ScriptConf &_conf) -> void
        {
            if (_entity.HasScript<SpaceInvaders::SwarmController>())
            {
                SwarmController& swarm = *_entity.GetScript<SwarmController>();
                ImGui::InputFloat(("horizontalSpeed##" + _conf.name).c_str(), &swarm.horizontalSpeed);
                ImGui::InputFloat(("stepDown##" + _conf.name).c_str(), &swarm.stepDown);
                ImGui::InputFloat(("fireInterval##" + _conf.name).c_str(), &swarm.fireInterval);
                ImGui::InputFloat(("enemyBulletSpeed##" + _conf.name).c_str(), &swarm.enemyBulletSpeed);
                ImGui::InputFloat(("enemyBulletLifetime##" + _conf.name).c_str(), &swarm.enemyBulletLifetime);
            }
        };

        _app.RegisterScript(conf);
    }

    DEFAULT_UNREGISTER_SCRIPT(conf, SwarmController)

    void SwarmController::Create() {}

    void SwarmController::Ready()
    {
        m_fireTimer = fireInterval;
    }

    void SwarmController::Destroy() {}

    void SwarmController::Update(float _dt)
    {
        std::vector<Entity*> enemies = entity.scene->GetEntitiesWithTag("Enemy");
        if (enemies.empty())
            return;

        const float halfW = entity.scene->GetWindow().GetScreenWidth() * 0.5f;
        const float moveDelta = horizontalSpeed * static_cast<float>(m_direction) * _dt;

        bool hitEdge = false;
        for (Entity* enemy : enemies)
        {
            if (enemy == nullptr || !enemy->active)
                continue;

            RectTransform* transform = enemy->GetComponent<RectTransform>();
            if (transform == nullptr)
                continue;

            const float halfSizeX = transform->size.x * transform->GetScale().x * 0.5f;
            const float nextX = transform->position.x + moveDelta;
            if (nextX + halfSizeX > (halfW - 12.0f) || nextX - halfSizeX < (-halfW + 12.0f))
            {
                hitEdge = true;
                break;
            }
        }

        for (Entity* enemy : enemies)
        {
            if (enemy == nullptr || !enemy->active)
                continue;

            RectTransform* transform = enemy->GetComponent<RectTransform>();
            if (transform == nullptr)
                continue;

            if (hitEdge)
                transform->position.y -= stepDown;
            else
                transform->position.x += moveDelta;

            if (transform->position.y < -160.0f)
            {
                if (Entity* controllerEntity = entity.scene->FindEntityWithName("GameController"))
                {
                    if (controllerEntity->HasScript<SpaceInvaders::GameController>())
                    {
                        GameController& controller = *controllerEntity->GetScript<SpaceInvaders::GameController>();
                        controller.SetGameOver("invaders reached player line");
                    }
                }
            }
        }

        if (hitEdge)
            m_direction *= -1;

        m_fireTimer -= _dt;
        if (m_fireTimer > 0.0f)
            return;

        m_fireTimer = fireInterval;

        std::vector<Entity*> shooters = {};
        shooters.reserve(enemies.size());
        for (Entity* enemy : enemies)
            if (enemy != nullptr && enemy->active && enemy->HasComponent<RectTransform>())
                shooters.push_back(enemy);

        if (shooters.empty())
            return;

        std::uniform_int_distribution<size_t> dist(0u, shooters.size() - 1u);
        Entity* shooter = shooters[dist(g_rng)];
        RectTransform* shooterTransform = shooter->GetComponent<RectTransform>();
        if (shooterTransform == nullptr)
            return;

        Entity* bulletEntity = entity.scene->CreateEntity("EnemyBullet", "EnemyBullet");
        RectTransform& bulletTransform = *bulletEntity->AddComponent<RectTransform>();
        Sprite2D& bulletSprite = *bulletEntity->AddComponent<Sprite2D>();
        Projectile* bullet = bulletEntity->AddScript<SpaceInvaders::Projectile>();

        bulletTransform.size = Vector2(16.0f, 24.0f);
        bulletTransform.position = shooterTransform->GetPosition() + Vector2(0.0f, -24.0f);

        bulletSprite.textureHandle = AssetManager::GetTextureHandle("assets/textures/awesome_face.png");
        bulletSprite.color = Color(1.0f, 0.45f, 0.45f, 1.0f);

        bullet->fromPlayer = false;
        bullet->velocity = Vector2(0.0f, -enemyBulletSpeed);
        bullet->lifeTime = enemyBulletLifetime;
    }

    void SwarmController::EditorInspectorDraw() {}
}
