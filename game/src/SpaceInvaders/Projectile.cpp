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
        REGISTER_PROPERTY(conf, SpaceInvaders::Projectile, velocity, Vector2);
        REGISTER_PROPERTY(conf, SpaceInvaders::Projectile, lifeTime, float);
        REGISTER_PROPERTY(conf, SpaceInvaders::Projectile, fromPlayer, bool);

        DEFAULT_CONFIG_AND_REQUIRED(conf, SpaceInvaders::Projectile, Canis::RectTransform, Canis::Sprite2D);

        conf.DrawInspector = [](Editor &, Entity &_entity, const ScriptConf &_conf) -> void
        {
            if (Projectile *projectile = CANIS_GET_SCRIPT(_entity, SpaceInvaders::Projectile))
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
        m_transform = CANIS_GET_COMPONENT(entity, RectTransform);
    }

    void Projectile::Destroy() {}

    void Projectile::Update(float _dt)
    {
        m_transform = CANIS_GET_COMPONENT(entity, RectTransform);
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

                RectTransform* enemyTransform = CANIS_GET_COMPONENT(enemy, RectTransform);
                if (enemyTransform == nullptr)
                    continue;

                if (!OverlapsAABB(*m_transform, *enemyTransform))
                    continue;

                int points = 10;
                if (Invader* invader = CANIS_GET_SCRIPT(enemy, SpaceInvaders::Invader))
                    points = invader->points;

                if (Entity* controllerEntity = entity.scene->FindEntityWithName("GameController"))
                {
                    if (GameController* controller = CANIS_GET_SCRIPT(controllerEntity, SpaceInvaders::GameController))
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
                    RectTransform* playerTransform = CANIS_GET_COMPONENT(playerEntity, RectTransform);
                    if (playerTransform != nullptr && OverlapsAABB(*m_transform, *playerTransform))
                    {
                        if (PlayerShip* playerShip = CANIS_GET_SCRIPT(playerEntity, SpaceInvaders::PlayerShip))
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
