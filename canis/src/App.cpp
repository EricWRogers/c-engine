#include <Canis/App.hpp>

#include <SDL3/SDL_loadso.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_filesystem.h>
#include <Canis/GameCodeObject.hpp>

namespace Canis
{
    void App::Run()
    {
        #ifdef Win32
        const char *sharedObjectPath = "./libGameCode.dll";
        #else
        const char *sharedObjectPath = "./libGameCode.so";
        #endif

        GameCodeObject gameCodeObject = GameCodeObjectInit(sharedObjectPath);
        GameCodeObjectInitFunction(&gameCodeObject, this);

        while (true)
        {
            // call the dynamically loaded function
            GameCodeObjectUpdateFunction(&gameCodeObject, this, 0.16f);
            GameCodeObjectWatchFile(&gameCodeObject, this);
        }

        gameCodeObject.GameShutdownFunction((void*)this, gameCodeObject.gameData);
        SDL_UnloadObject(gameCodeObject.sharedObjectHandle);
    }
}