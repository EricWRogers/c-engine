#pragma once
#define GLM_ENABLE_EXPERIMENTAL

#include <SDL3/SDL.h>
#include <glm/glm.hpp>

#ifdef _WIN32
#  ifdef CANISENGINE_EXPORTS
#    define CANIS_API __declspec(dllexport)
#  else
#    define CANIS_API __declspec(dllimport)
#  endif
#else
#  define CANIS_API
#endif

namespace Canis
{
    struct ProjectConfig
    {
        bool fullscreen = false;
        bool borderless = false;
        bool resizeable = false;
        int width = 1280;
        int heigth = 720;
        bool useFrameLimit = false;
        int frameLimit = 60;
        bool overrideSeed = false;
        unsigned int seed = 0;
        float volume = 1.0f;
        float musicVolume = 1.0f;
        float sfxVolume = 1.0f;
        bool mute = false;
        bool log = true;
        bool logToFile = false;
        bool editor = false;
        bool vsync = false;
    };

    struct CanisData {
        void *sdlWindow = nullptr;
        void *glContext = nullptr;
        bool resized = false;
        int screenWidth, screenHeight;
        bool fullscreen = false;
        bool mouseLock = false;
        glm::vec4 clearColor;
        float fps = 0.0f;
        void* frameRateManager = nullptr;
    };

    ProjectConfig& GetProjectConfig();
    CanisData& GetCanisData();
    bool SaveProjectConfig();

    CANIS_API int Init();
    CANIS_API void Start();
} // end of Canis namespace
