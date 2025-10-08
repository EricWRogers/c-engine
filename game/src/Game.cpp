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

#include <Canis/ECS/Systems/SpriteRenderer2DSystem.hpp>

#include <GameData.hpp>

using namespace Canis;

class GameScript : public Canis::ScriptableEntity {
  public:
    int id = 5;
    int counter = 0;
    float amplitude = 20.0f;
    Vector2 direction = Vector2(1.0f, 0.5f);
    float speed = 200.0f;

    Canis::Sprite2D& sprite = *entity.GetScript<Canis::Sprite2D>();

    GameScript(Canis::Entity& _entity) : Canis::ScriptableEntity(_entity) {}

    void Create() {
        Canis::Time::SetTargetFPS(30.0f);
        direction = Vector2(1.0f, 0.5f).Normalize();
        direction = Vector2::Normalize(direction);
    }

    void Ready() { Canis::Debug::Log("OnReady"); }

    void Destroy() { Canis::Debug::Log("OnDestroy"); }

    void Update(float _dt) {
        Vector2 delta = direction * speed * 6.0f * Time::DeltaTime();
        sprite.position += delta;

        CheckWalls();
        Canis::Debug::Log(
            "Game Script update %.2f %d Counter %d FPS: %f rect.x: %f", _dt, id,
            counter++, Canis::Time::FPS(), sprite.position.x);
    }

    void CheckWalls() {
        Canis::Window *window = entity.scene->GetWindow();

        if (window->GetScreenWidth() * 0.5f <=
            sprite.position.x + sprite.size.x * 0.5f) {
            if (direction.x > 0.0f) {
                direction.x *= -1.0f;
            }
        } else if (-window->GetScreenWidth() * 0.5f >=
                   sprite.position.x - sprite.size.x * 0.5f) {
            if (direction.x < 0.0f) {
                direction.x *= -1.0f;
            }
        } else if (window->GetScreenHeight() * 0.5f <=
                   sprite.position.y + sprite.size.y * 0.5f) {
            if (direction.y > 0.0f) {
                direction.y *= -1.0f;
            }
        } else if (-window->GetScreenHeight() * 0.5f >=
                   sprite.position.y - sprite.size.y * 0.5f) {
            if (direction.y < 0.0f) {
                direction.y *= -1.0f;
            }
        }
    }
};

extern "C" {
void *GameInit(void *_app) {
    Canis::App &app = *(Canis::App *)_app;

    // TODO : fix following line
    // app.scene.CreateRenderSystem<Canis::SpriteRenderer2DSystem>();

    Canis::Entity *cEntity = app.scene.CreateEntity();
    Canis::Camera2D *camera2D = cEntity->AddScript<Canis::Camera2D>();

    Canis::Entity *entityOne = app.scene.CreateEntity();
    Canis::Sprite2D *sprite = entityOne->AddScript<Canis::Sprite2D>();
    entityOne->AddScript<GameScript>();

    sprite->textureHandle = Canis::AssetManager::GetTextureHandle(
        "assets/textures/awesome_face.png");
    sprite->size = Vector2(32.0f);

    Canis::Debug::Log("Game initialized!");
    GameData *gameData = (GameData *)malloc(sizeof(GameData));
    *gameData = GameData{};
    gameData->id = 15;
    return (void *)gameData;
}

void GameUpdate(void *_app, float dt, void *_data) {
    Canis::App &app = *(Canis::App *)_app;
    GameData &gameData = *(GameData *)_data;
    // Canis::Debug::Log("Game update %.2f %d Counter %d", dt, gameData.id,
    // gameData.counter++);
}

void GameShutdown(void *_app, void *_data) {
    Canis::App &app = *(Canis::App *)_app;
    app.scene.Unload();
    Canis::Debug::Log("Game shutdown!");
    delete (GameData *)_data;
}
}
