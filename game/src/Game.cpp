#include "../include/Game.hpp"

#include <Canis/App.hpp>
#include <Canis/AssetManager.hpp>
#include <Canis/Time.hpp>
#include <Canis/Window.hpp>
#include <Canis/InputManager.hpp>

#include "../include/Pong/Ball.hpp"
#include "../include/Pong/Paddle.hpp"
#include <TankGame/Tank.hpp>

#include "../include/GameData.hpp"

using namespace Canis;

extern "C"
{
    void SpawnAwesome(Canis::App &_app);

    InspectorItemRightClick inspectorCreateBall = {
        .name = "Create Ball",
        .Func = [](App& _app, Editor& _editor, Entity& _entity, std::vector<ScriptConf>& _scriptConfs) -> void {
            SpawnAwesome(_app);
        }
    };

    void *GameInit(void *_app)
    {
        Canis::App &app = *(Canis::App *)_app;
        
        app.RegisterInspectorItem(inspectorCreateBall);
        TankGame::RegisterTankScript(app);
        Pong::RegisterBallScript(app);
        Pong::RegisterPaddleScript(app);
        

        Canis::Debug::Log("Game initialized!");
        GameData *gameData = (GameData *)malloc(sizeof(GameData));
        *gameData = GameData{};
        gameData->id = 5;
        return (void *)gameData;
    }

    void GameUpdate(void *_app, float dt, void *_data)
    {
        Canis::App &app = *(Canis::App *)_app;
        GameData &gameData = *(GameData *)_data;
    }

    void GameShutdown(void *_app, void *_data)
    {
        Canis::App &app = *(Canis::App *)_app;

        app.UnregisterInspectorItem(inspectorCreateBall);
        Pong::UnRegisterBallScript(app);
        Pong::UnRegisterPaddleScript(app);
        TankGame::UnRegisterTankScript(app);

        Canis::Debug::Log("Game shutdown!");
        delete (GameData *)_data;
    }

    void SpawnAwesome(Canis::App &_app)
    {
        Canis::Entity *entityOne = _app.scene.CreateEntity("Ball");
        Canis::RectTransform * transform = entityOne->AddScript<Canis::RectTransform>();
        Canis::Sprite2D *sprite = entityOne->AddScript<Canis::Sprite2D>();
        entityOne->AddScript<Pong::Ball>();

        sprite->textureHandle = Canis::AssetManager::GetTextureHandle("assets/textures/awesome_face.png");
        transform->size = Vector2(32.0f);
    }
}
