#include <stdio.h>
#include <stdlib.h> 
#include <SDL3/SDL_log.h>

#include <Canis/App.hpp>
#include <Canis/GameCodeObject.hpp>

struct GameData {
    int id = 5;
    int counter = 0;
};

class Game : public Canis::ScriptableEntity {
public:
    int id = 5;
    int counter = 0;

    void OnCreate()
    {
        SDL_Log("OnCreate");
    }

    void OnReady()
    {
        SDL_Log("OnReady");
    }

    void OnDestroy()
    {
        SDL_Log("OnDestroy");
    }

    void OnUpdate(float _dt)
    {
        SDL_Log("OnUpdate");
        SDL_Log("Yo Game Script update %.2f %d Counter %d", _dt, id, counter++);
    }
};

extern "C" {
void* GameInit(void* _app) {
    Canis::App& app = *(Canis::App*)_app;

    Canis::Entity* entityOne = app.scene.CreateEntity();
    entityOne->AddScript<Game>();

    SDL_Log("Game initialized!");
    GameData* gameData = (GameData*)malloc(sizeof(GameData));
    *gameData = GameData{};
    gameData->id = 15;
    return (void*) gameData;
}

void GameUpdate(void* _app, float dt, void* _data) {
    Canis::App& app = *(Canis::App*)_app;
    GameData& gameData = *(GameData*)_data;
    SDL_Log("Game update %.2f %d Counter %d", dt, gameData.id, gameData.counter++);
}

void GameShutdown(void* _app, void* _data) {
    Canis::App& app = *(Canis::App*)_app;
    SDL_Log("Game shutdown");
    SDL_free(_data);
}

}
