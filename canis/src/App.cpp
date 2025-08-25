#include <Canis/App.hpp>

#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_loadso.h>

#include <Canis/GameCodeObject.hpp>
#include <Canis/Time.hpp>
#include <Canis/Window.hpp>

namespace Canis
{
    void App::Run()
    {
#ifdef Win32
        const char *sharedObjectPath = "./libGameCode.dll";
#elif __APPLE__
        const char *sharedOpbjectPath = "./libGameCode.dylib";
#else
        const char *sharedObjectPath = "./libGameCode.so";
#endif

        // init window
        Window window("Canis Beta", 512, 512);

        Time::Init(120.0f);

        scene.Init(this, &window);

        GameCodeObject gameCodeObject = GameCodeObjectInit(sharedObjectPath);
        GameCodeObjectInitFunction(&gameCodeObject, this);
        

        while (!window.ShouldClose())
        {
            window.PollEvents();
            window.Clear(0.1f, 0.1f, 0.1f, 1.0f);

            float deltaTime = Time::StartFrame();
            scene.Update(deltaTime);
            // call the dynamically loaded function
            GameCodeObjectUpdateFunction(&gameCodeObject, this, deltaTime);
            GameCodeObjectWatchFile(&gameCodeObject, this);

            window.Render();
            window.Display();

            Time::EndFrame();
        }

        scene.Unload();
        Time::Quit();
        gameCodeObject.GameShutdownFunction((void *)this, gameCodeObject.gameData);
        SDL_UnloadObject(gameCodeObject.sharedObjectHandle);
    }

    float App::FPS()
    {
        return Time::FPS();
    }

    float App::DeltaTime()
    {
        return Time::DeltaTime();
    }

    void App::SetTargetFPS(float _targetFPS)
    {
        Time::SetTargetFPS(_targetFPS);
    }

} // namespace Canis
