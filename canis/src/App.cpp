#include <Canis/App.hpp>

#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_loadso.h>

#include <Canis/GameCodeObject.hpp>
#include <Canis/Time.hpp>
#include <Canis/Window.hpp>

namespace Canis {
void App::Run() {
#ifdef Win32
  const char *sharedObjectPath = "./libGameCode.dll";
#elif __APPLE__
  const char *sharedOpbjectPath = "./libGameCode.dylib";
#else
  const char *sharedObjectPath = "./libGameCode.so";
#endif

  Window window("Canis Beta", 400, 240);

  GameCodeObject gameCodeObject = GameCodeObjectInit(sharedObjectPath);
  GameCodeObjectInitFunction(&gameCodeObject, this);
  Time::Init(60.0f);
  scene.Init();

  while (!window.ShouldClose()) {
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
} // namespace Canis
