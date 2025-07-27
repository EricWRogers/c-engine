#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_loadso.h>
#include <SDL3/SDL_error.h>

int main(int argc, char* argv[]) {
    
    // load gameplay shared library
    SDL_SharedObject* handle = SDL_LoadObject("./libGameCode.so");
    if (handle == NULL) {
        SDL_Log("Error loading shared object: %s", SDL_GetError());
        return 1;
    }

    // load the exported function from mylib.so
    void* (*GameInitFunction)();
    const char* gameInitName = "GameInit";
    GameInitFunction = (void* (*)())SDL_LoadFunction(handle, gameInitName);
    if (GameInitFunction == NULL) {
        SDL_Log("Error loading function '%s': %s", gameInitName, SDL_GetError());
        SDL_UnloadObject(handle);
        return 1;
    }

    void (*GameUpdateFunction)(float, void*);
    const char* gameUpdateName = "GameUpdate";
    GameUpdateFunction = (void (*)(float, void*))SDL_LoadFunction(handle, gameUpdateName);
    if (GameUpdateFunction == NULL) {
        SDL_Log("Error loading function '%s': %s", gameUpdateName, SDL_GetError());
        SDL_UnloadObject(handle);
        return 1;
    }

    void (*GameShutdownFunction)(void*);
    const char* gameShutdownName = "GameShutdown";
    GameShutdownFunction = (void (*)(void*))SDL_LoadFunction(handle, gameShutdownName);
    if (GameShutdownFunction == NULL) {
        SDL_Log("Error loading function '%s': %s", gameShutdownName, SDL_GetError());
        SDL_UnloadObject(handle);
        return 1;
    }

    // call the dynamically loaded function
    void* gameData = GameInitFunction();
    GameUpdateFunction(0.16f, gameData);
    GameShutdownFunction(gameData);

    SDL_UnloadObject(handle);

    return 0;
}