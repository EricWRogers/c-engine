#include <stdio.h>
#include <stdlib.h> 
#include <SDL3/SDL_log.h>
extern "C" {

struct GameData {
    int id = 5;
};

void* GameInit() {
    SDL_Log("Game initialized!");
    GameData* gameData = (GameData*)malloc(sizeof(GameData));
    gameData->id = 5;
    return (void*) gameData;
}

void GameUpdate(float dt, void* _data) {
    GameData& gameData = *(GameData*)_data;
    SDL_Log("Game update %.2f %d", dt, gameData.id);
}

void GameShutdown(void* _data) {
    SDL_Log("Game shutdown");
    SDL_free(_data);
}

}
