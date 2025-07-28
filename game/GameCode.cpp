#include <stdio.h>
#include <stdlib.h> 
#include <SDL3/SDL_log.h>

#include <Canis/App.hpp>
#include <Canis/GameCodeObject.hpp>

struct GameData {
    int id = 5;
    int counter = 0;
};

extern "C" {
void* GameInit() {
    SDL_Log("Game initialized!");
    GameData* gameData = (GameData*)malloc(sizeof(GameData));
    *gameData = GameData{};
    return (void*) gameData;
}

void GameUpdate(float dt, void* _data) {
    GameData& gameData = *(GameData*)_data;
    SDL_Log("Game update %.2f %d Counter %d", dt, gameData.id, gameData.counter++);
}

void GameShutdown(void* _data) {
    SDL_Log("Game shutdown");
    SDL_free(_data);
}

}
