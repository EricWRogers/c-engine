#include <Canis/App.hpp>

#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_loadso.h>

#include <Canis/GameCodeObject.hpp>
#include <Canis/Time.hpp>
#include <Canis/Debug.hpp>
#include <Canis/Window.hpp>
#include <Canis/Editor.hpp>
#include <Canis/InputManager.hpp>
#include <Canis/ECS/Systems/SpriteRenderer2DSystem.hpp>

namespace Canis
{
    void App::Run()
    {
        Debug::Log("App Run");
#ifdef Win32
        const char *sharedObjectPath = "./libGameCode.dll";
#elif __APPLE__
        const char *sharedOpbjectPath = "./libGameCode.dylib";
#else
        const char *sharedObjectPath = "./libGameCode.so";
#endif

        // init window
        Window window("Canis Beta", 512, 512);

        Editor editor;
        editor.Init(&window);

        InputManager inputManager;

        Time::Init(1200.0f);

        scene.Init(this, &window, &inputManager);

        scene.CreateRenderSystem<Canis::SpriteRenderer2DSystem>();

        scene.Load(); // call after all the systems are added

        GameCodeObject gameCodeObject = GameCodeObjectInit(sharedObjectPath);
        GameCodeObjectInitFunction(&gameCodeObject, this);

        while (inputManager.Update((void *)&window))
        {
            window.Clear(1.0f, 1.0f, 1.0f, 1.0f);

            float deltaTime = Time::StartFrame();
            scene.Update(deltaTime);

            // call the dynamically loaded function
            GameCodeObjectUpdateFunction(&gameCodeObject, this, deltaTime);
            GameCodeObjectWatchFile(&gameCodeObject, this);

            scene.Render(deltaTime);
            editor.Draw(&scene, &window);
            window.SwapBuffer();

            

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
