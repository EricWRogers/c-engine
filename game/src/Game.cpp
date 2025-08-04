#include <Game.hpp>

#include <stdio.h>
#include <stdlib.h> 

#include <Canis/App.hpp>
#include <Canis/Time.hpp>
#include <Canis/Debug.hpp>
#include <Canis/GameCodeObject.hpp>

struct GameData {
    int id = 5;
    int counter = 0;
};

class GameScript : public Canis::ScriptableEntity {
public:
    int id = 5;
    int counter = 0;

    void OnCreate()
    {
        Canis::Debug::Log("OnCreate");
        Canis::Time::SetTargetFPS(60.0f);
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
        Canis::Debug::Log("OnUpdate");
        Canis::Debug::Log("Yo Game Script update %.2f %d Counter %d FPS: %f", _dt, id, counter++, Canis::Time::FPS());
    }
};

extern "C" {
void* GameInit(void* _app) {
    Canis::App& app = *(Canis::App*)_app;

    Canis::Entity* entityOne = app.scene.CreateEntity();
    entityOne->AddScript<GameScript>();

    Canis::Debug::Log("Game initialized!");
    GameData* gameData = (GameData*)malloc(sizeof(GameData));
    *gameData = GameData{};
    gameData->id = 15;
    return (void*) gameData;
}

void  GameUpdate(void* _app, float dt, void* _data) {
    Canis::App& app = *(Canis::App*)_app;
    GameData& gameData = *(GameData*)_data;
    Canis::Debug::Log("Game update %.2f %d Counter %d", dt, gameData.id, gameData.counter++);
}

void  GameShutdown(void* _app, void* _data) {
    Canis::App& app = *(Canis::App*)_app;
    app.scene.Unload();
    Canis::Debug::Log("Game shutdown!");
    delete (GameData*)_data;
}
}
