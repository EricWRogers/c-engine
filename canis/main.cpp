#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_gpu.h>

#include <Canis/App.hpp>

int main(int argc, char *argv[])
{
    Canis::App app;

    app.Run();

    return 0;
}