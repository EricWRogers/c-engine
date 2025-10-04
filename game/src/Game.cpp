#include <Game.hpp>

#include <stdio.h>
#include <stdlib.h>

#include <Canis/App.hpp>
#include <Canis/Time.hpp>
#include <Canis/Debug.hpp>
#include <Canis/Entity.hpp>
#include <Canis/AssetManager.hpp>
#include <Canis/GameCodeObject.hpp>

#include <Canis/ECS/Systems/SpriteRenderer2DSystem.hpp>

#include <GameData.hpp>

class GameScript : public Canis::ScriptableEntity
{
public:
    int id = 5;
    int counter = 0;
    

    void OnCreate()
    {
        //Canis::Sprite2D& sprite = *entity->GetScript<Canis::Sprite2D>();
        //Canis::Debug::Log("OnCreate");
        //Canis::Time::SetTargetFPS(30.0f);

        //sprite.position.x = 10.0f;
    }

    void OnReady()
    {
        Canis::Debug::Log("OnReady");
    }

    void OnDestroy()
    {
        Canis::Debug::Log("OnDestroy");
    }

    void OnUpdate(float _dt)
    {
        //Canis::Sprite2D& sprite = *entity->GetScript<Canis::Sprite2D>();
        //Canis::Debug::Log("Game Script update %.2f %d Counter %d FPS: %f rect.x: %f", _dt, id, counter++, Canis::Time::FPS(), sprite.position.x++);
    }
};

extern "C"
{
    void *GameInit(void *_app)
    {
        Canis::App &app = *(Canis::App *)_app;

        //app.scene.CreateRenderSystem<Canis::SpriteRenderer2DSystem>();

        Canis::Entity *entityOne = app.scene.CreateEntity();
        Canis::Sprite2D* sprite = entityOne->AddScript<Canis::Sprite2D>();
        //entityOne->AddScript<GameScript>();

        sprite->textureHandle = Canis::AssetManager::GetTextureHandle("assets/textures/awesome_face.png");
        sprite->size = Vector2(32.0f);

        Canis::Debug::Log("Game initialized!");
        GameData *gameData = (GameData *)malloc(sizeof(GameData));
        *gameData = GameData{};
        gameData->id = 15;
        return (void *)gameData;
    }

    void GameUpdate(void *_app, float dt, void *_data)
    {
        Canis::App &app = *(Canis::App *)_app;
        GameData &gameData = *(GameData *)_data;
        // Canis::Debug::Log("Game update %.2f %d Counter %d", dt, gameData.id, gameData.counter++);
    }

    void GameShutdown(void *_app, void *_data)
    {
        Canis::App &app = *(Canis::App *)_app;
        app.scene.Unload();
        Canis::Debug::Log("Game shutdown!");
        delete (GameData *)_data;
    }
}
