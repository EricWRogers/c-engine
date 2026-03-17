#include <SpaceInvaders/Projectile.hpp>

#include <SpaceInvaders/GameController.hpp>
#include <SpaceInvaders/Invader.hpp>
#include <SpaceInvaders/PlayerShip.hpp>

#include <Canis/App.hpp>
#include <Canis/Scene.hpp>
#include <Canis/Window.hpp>
#include <Canis/ConfigHelper.hpp>

#include <cmath>

using namespace Canis;

namespace SpaceInvaders
{
    namespace
    {
        ScriptConf conf = {};

        bool OverlapsAABB(const RectTransform& _a, const RectTransform& _b)
        {
            const Vector2 aPos = _a.GetPosition();
            const Vector2 bPos = _b.GetPosition();
            const Vector2 aHalf = (_a.size * _a.GetScale()) * 0.5f;
            const Vector2 bHalf = (_b.size * _b.GetScale()) * 0.5f;

            return std::abs(aPos.x - bPos.x) <= (aHalf.x + bHalf.x) &&
                std::abs(aPos.y - bPos.y) <= (aHalf.y + bHalf.y);
        }
    }

    void RegisterProjectileScript(Canis::App &_app)
    {
        REGISTER_PROPERTY(conf, SpaceInvaders::Projectile, velocity);
        REGISTER_PROPERTY(conf, SpaceInvaders::Projectile, lifeTime);
        REGISTER_PROPERTY(conf, SpaceInvaders::Projectile, fromPlayer);

        DEFAULT_CONFIG_AND_REQUIRED(conf, SpaceInvaders::Projectile, Canis::RectTransform, Canis::Sprite2D);

        conf.DrawInspector = [](Editor &, Entity &_entity, const ScriptConf &_conf) -> void
        {
            Projectile* projectile = _entity.GetScript<SpaceInvaders::Projectile>();
            if (projectile != nullptr)
            {
                ImGui::InputFloat2(("velocity##" + _conf.name).c_str(), &projectile->velocity.x);
                ImGui::InputFloat(("lifeTime##" + _conf.name).c_str(), &projectile->lifeTime);
                ImGui::Checkbox(("fromPlayer##" + _conf.name).c_str(), &projectile->fromPlayer);
            }
        };

        _app.RegisterScript(conf);
    }

    DEFAULT_UNREGISTER_SCRIPT(conf, Projectile)

    void Projectile::Create() {}

    void Projectile::Ready()
    {
        m_transform = entity.GetComponent<RectTransform>();
    }

    void Projectile::Destroy() {}

    void Projectile::Update(float _dt)
    {
        m_transform = entity.GetComponent<RectTransform>();
        if (m_transform == nullptr)
            return;

        m_transform->Move(velocity * _dt);

        lifeTime -= _dt;
        if (lifeTime <= 0.0f)
        {
            entity.Destroy();
            return;
        }

        const float halfW = entity.scene->GetWindow().GetScreenWidth() * 0.5f + 64.0f;
        const float halfH = entity.scene->GetWindow().GetScreenHeight() * 0.5f + 64.0f;
        const Vector2 p = m_transform->GetPosition();
        if (p.x < -halfW || p.x > halfW || p.y < -halfH || p.y > halfH)
        {
            entity.Destroy();
            return;
        }

        if (fromPlayer)
        {
            std::vector<Entity*> enemies = entity.scene->GetEntitiesWithTag("Enemy");
            for (Entity* enemy : enemies)
            {
                if (enemy == nullptr || !enemy->active)
                    continue;

                RectTransform* enemyTransform = enemy->GetComponent<RectTransform>();
                if (enemyTransform == nullptr)
                    continue;

                if (!OverlapsAABB(*m_transform, *enemyTransform))
                    continue;

                int points = 10;
                Invader* invader = enemy->GetScript<SpaceInvaders::Invader>();
                if (invader != nullptr)
                    points = invader->points;

                if (Entity* controllerEntity = entity.scene->FindEntityWithName("GameController"))
                {
                    GameController* controller = controllerEntity->GetScript<SpaceInvaders::GameController>();
                    if (controller != nullptr)
                        controller->AddScore(points);
                }

                enemy->Destroy();
                entity.Destroy();
                return;
            }
        }
        else
        {
            if (Entity* playerEntity = entity.scene->GetEntityWithTag("Player"))
            {
                if (playerEntity->active)
                {
                    RectTransform* playerTransform = playerEntity->GetComponent<RectTransform>();
                    if (playerTransform != nullptr && OverlapsAABB(*m_transform, *playerTransform))
                    {
                        PlayerShip* playerShip = playerEntity->GetScript<SpaceInvaders::PlayerShip>();
                        if (playerShip != nullptr)
                            playerShip->OnHit();

                        entity.Destroy();
                        return;
                    }
                }
            }
        }
    }

    void Projectile::EditorInspectorDraw() {}
}
