#include "../include/Game.hpp"

#include <Canis/App.hpp>
#include <Canis/AssetManager.hpp>
#include <Canis/Time.hpp>
#include <Canis/Window.hpp>
#include <Canis/InputManager.hpp>

#include <TankGame/Tank.hpp>
#include <TankGame/Bullet.hpp>
#include <TankGame/FollowMouse.hpp>
#include <TankGame/Bounce.hpp>
#include <RollABall/PlayerController.hpp>
#include <RollABall/PickupSpinner.hpp>

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
        TankGame::RegisterBulletScript(app);
        TankGame::RegisterFollowMouseScript(app);
        TankGame::RegisterBounceScript(app);
        RollABall::RegisterPlayerControllerScript(app);
        RollABall::RegisterPickupSpinnerScript(app);
        

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
        TankGame::UnRegisterTankScript(app);
        TankGame::UnRegisterBulletScript(app);
        TankGame::UnRegisterFollowMouseScript(app);
        TankGame::UnRegisterBounceScript(app);
        RollABall::UnRegisterPickupSpinnerScript(app);
        RollABall::UnRegisterPlayerControllerScript(app);

        Canis::Debug::Log("Game shutdown!");
        delete (GameData *)_data;
    }

    void SpawnAwesome(Canis::App &_app)
    {
        Canis::Entity *entityOne = _app.scene.CreateEntity("Ball");
        RectTransform& transform = *entityOne->AddComponent<RectTransform>();
        Sprite2D& sprite = *entityOne->AddComponent<Sprite2D>();

        sprite.textureHandle = Canis::AssetManager::GetTextureHandle("assets/textures/awesome_face.png");
        transform.size = Vector2(32.0f);
    }
}
