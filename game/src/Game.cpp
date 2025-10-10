#include <Game.hpp>

#include <stdio.h>
#include <stdlib.h>

#include <Canis/App.hpp>
#include <Canis/AssetManager.hpp>
#include <Canis/Debug.hpp>
#include <Canis/Entity.hpp>
#include <Canis/GameCodeObject.hpp>
#include <Canis/Time.hpp>
#include <Canis/Window.hpp>
#include <Canis/InputManager.hpp>

#include <Canis/ECS/Systems/SpriteRenderer2DSystem.hpp>

#include <GameData.hpp>

using namespace Canis;

class GameScript : public Canis::ScriptableEntity
{
public:
    int id = 5;
    int counter = 0;
    float amplitude = 20.0f;
    Vector2 direction = Vector2(1.0f, 0.5f);
    float speed = 100.0f;

    Canis::Sprite2D &sprite = *entity.GetScript<Canis::Sprite2D>();

    GameScript(Canis::Entity &_entity) : Canis::ScriptableEntity(_entity) {}

    void Create()
    {
        Canis::Time::SetTargetFPS(30.0f);
        direction = Vector2(1.0f, 0.5f).Normalize();
        direction = Vector2::Normalize(direction);
    }

    void Ready() { Canis::Debug::Log("OnReady"); }

    void Destroy() { Canis::Debug::Log("OnDestroy"); }

    void Update(float _dt)
    {
        Vector2 delta = direction * speed * Time::DeltaTime();
        sprite.position += delta;

        CheckWalls();
    }

    void CheckWalls()
    {
        Canis::Window &window = entity.scene->GetWindow();

        if (window.GetScreenWidth() * 0.5f <= sprite.position.x + sprite.size.x * 0.5f)
        {
            if (direction.x > 0.0f)
            {
                direction.x *= -1.0f;
            }
        }
        else if (-window.GetScreenWidth() * 0.5f >= sprite.position.x - sprite.size.x * 0.5f)
        {
            if (direction.x < 0.0f)
            {
                direction.x *= -1.0f;
            }
        }
        else if (window.GetScreenHeight() * 0.5f <= sprite.position.y + sprite.size.y * 0.5f)
        {
            if (direction.y > 0.0f)
            {
                direction.y *= -1.0f;
            }
        }
        else if (-window.GetScreenHeight() * 0.5f >= sprite.position.y - sprite.size.y * 0.5f)
        {
            if (direction.y < 0.0f)
            {
                direction.y *= -1.0f;
            }
        }
    }
};

extern "C"
{
    void SpawnCamera(Canis::App &_app);
    void SpawnAwesome(Canis::App &_app);
    void ReloadScene(Canis::App &_app);

    void *GameInit(void *_app)
    {
        Canis::App &app = *(Canis::App *)_app;

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

        app.SetTargetFPS(100000);

        if (app.scene.GetInputManager().JustPressedKey(Canis::Key::C))
        {
            SpawnCamera(app);
        }

        // if (app.scene.GetInputManager().mouseRel != VECTOR2_ZERO)
        if (app.scene.GetInputManager().JustPressedKey(Canis::Key::B))
        {
            SpawnAwesome(app);
        }

        if (app.scene.GetInputManager().JustPressedKey(Canis::Key::SPACE))
        {
            ReloadScene(app);
        }

        //Canis::Debug::Log("Update FPS: %f", Canis::Time::FPS());
    }

    void GameShutdown(void *_app, void *_data)
    {
        Canis::App &app = *(Canis::App *)_app;

        Canis::Debug::Log("Game shutdown!");
        delete (GameData *)_data;
    }

    void SpawnCamera(Canis::App &_app)
    {
        Canis::Entity *cEntity = _app.scene.CreateEntity();
        Canis::Camera2D *camera2D = cEntity->AddScript<Canis::Camera2D>();
    }

    void SpawnAwesome(Canis::App &_app)
    {
        Canis::Entity *entityOne = _app.scene.CreateEntity();
        Canis::Sprite2D *sprite = entityOne->AddScript<Canis::Sprite2D>();
        entityOne->AddScript<GameScript>();

        sprite->textureHandle = Canis::AssetManager::GetTextureHandle("assets/textures/awesome_face.png");
        sprite->size = Vector2(32.0f);
    }

    void ReloadScene(Canis::App &_app)
    {
        _app.scene.Unload();
        _app.scene.CreateRenderSystem<Canis::SpriteRenderer2DSystem>();
        _app.scene.Load(); // call after all the systems are added

        SpawnCamera(_app);
        SpawnAwesome(_app);
    }
}
