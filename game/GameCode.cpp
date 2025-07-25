#include <SDL3/SDL_log.h>
extern "C" {

void GameInit() {
    SDL_Log("Game initialized!");
}

void GameUpdate(float dt) {
    SDL_Log("Game update %.2f", dt);
}

void GameShutdown() {
    SDL_Log("Game shutdown");
}

}
