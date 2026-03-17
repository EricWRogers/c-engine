#include <SpaceInvaders/GameController.hpp>
#include <SpaceInvaders/SwarmController.hpp>
#include <SpaceInvaders/Invader.hpp>
#include <SpaceInvaders/PlayerShip.hpp>

#include <Canis/App.hpp>
#include <Canis/Scene.hpp>
#include <Canis/InputManager.hpp>
#include <Canis/Debug.hpp>
#include <Canis/ConfigHelper.hpp>

using namespace Canis;

namespace SpaceInvaders
{
    namespace
    {
        ScriptConf conf = {};
    }

    void RegisterGameControllerScript(Canis::App &_app)
    {
        REGISTER_PROPERTY(conf, SpaceInvaders::GameController, score, int);
        REGISTER_PROPERTY(conf, SpaceInvaders::GameController, lives, int);
        REGISTER_PROPERTY(conf, SpaceInvaders::GameController, gameOver, bool);
        REGISTER_PROPERTY(conf, SpaceInvaders::GameController, levelCleared, bool);

        DEFAULT_CONFIG(conf, SpaceInvaders::GameController);

        conf.DrawInspector = [](Editor &, Entity &_entity, const ScriptConf &_conf) -> void
        {
            GameController* controller = _entity.GetScript<SpaceInvaders::GameController>();
            if (controller != nullptr)
            {
                ImGui::InputInt(("score##" + _conf.name).c_str(), &controller->score);
                ImGui::InputInt(("lives##" + _conf.name).c_str(), &controller->lives);
                ImGui::Checkbox(("gameOver##" + _conf.name).c_str(), &controller->gameOver);
                ImGui::Checkbox(("levelCleared##" + _conf.name).c_str(), &controller->levelCleared);
            }
        };

        _app.RegisterScript(conf);
    }

    DEFAULT_UNREGISTER_SCRIPT(conf, GameController)

    void GameController::Create() {}
    void GameController::Ready()
    {
        if (!entity.HasScript<SpaceInvaders::SwarmController>())
            (void)entity.AddScript<SpaceInvaders::SwarmController>();

        if (Entity* player = entity.scene->GetEntityWithTag("Player"))
        {
            if (!player->HasScript<SpaceInvaders::PlayerShip>())
                (void)player->AddScript<SpaceInvaders::PlayerShip>();
        }

        std::vector<Entity*> enemies = entity.scene->GetEntitiesWithTag("Enemy");
        for (Entity* enemy : enemies)
        {
            if (enemy == nullptr)
                continue;

            if (!enemy->HasScript<SpaceInvaders::Invader>())
                (void)enemy->AddScript<SpaceInvaders::Invader>();

            Invader* invader = enemy->GetScript<SpaceInvaders::Invader>();
            if (invader != nullptr)
                invader->points = (enemy->name == "UFO") ? 50 : 10;
        }
    }
    void GameController::Destroy() {}

    void GameController::Update(float)
    {
        if (!gameOver && !levelCleared)
        {
            const std::vector<Entity*> enemies = entity.scene->GetEntitiesWithTag("Enemy");
            if (enemies.empty())
            {
                levelCleared = true;
                Debug::Log("SpaceInvaders: Level cleared! Final score: %d", score);
            }
        }

        if (gameOver && entity.scene->GetInputManager().JustPressedKey(Canis::Key::R))
        {
            score = 0;
            lives = 3;
            gameOver = false;
            levelCleared = false;
            Debug::Log("SpaceInvaders: Reset game state (reload scene to rebuild entities).");
        }
    }

    void GameController::EditorInspectorDraw() {}

    void GameController::AddScore(int _value)
    {
        score += _value;
    }

    void GameController::OnPlayerHit()
    {
        if (gameOver)
            return;

        lives -= 1;
        Debug::Log("SpaceInvaders: Player hit. Lives: %d", lives);

        if (lives <= 0)
            SetGameOver("out of lives");
    }

    void GameController::SetGameOver(const char* _reason)
    {
        if (gameOver)
            return;

        gameOver = true;
        if (_reason != nullptr)
            Debug::Log("SpaceInvaders: Game over (%s). Score: %d", _reason, score);
        else
            Debug::Log("SpaceInvaders: Game over. Score: %d", score);
    }
}
