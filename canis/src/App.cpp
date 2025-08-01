#include <Canis/App.hpp>

#include <SDL3/SDL_loadso.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_filesystem.h>

#include <Canis/Time.hpp>
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
        Time::Init(60.0f);
        scene.Init();

        while (true)
        {
            float deltaTime = Time::StartFrame();
            scene.Update(deltaTime);
            // call the dynamically loaded function
            GameCodeObjectUpdateFunction(&gameCodeObject, this, deltaTime);
            GameCodeObjectWatchFile(&gameCodeObject, this);
            Time::EndFrame();
        }

        scene.Unload();
        Time::Quit();
        gameCodeObject.GameShutdownFunction((void*)this, gameCodeObject.gameData);
        SDL_UnloadObject(gameCodeObject.sharedObjectHandle);
    }
}