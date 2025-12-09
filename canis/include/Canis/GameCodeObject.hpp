#pragma once
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_loadso.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_filesystem.h>

#include <Canis/Debug.hpp>

namespace Canis
{
    struct GameCodeObject
    {
        SDL_SharedObject *sharedObjectHandle;
        const char *path;
        SDL_PathInfo pathInfo;
        void *gameData;
        void *(*GameInitFunction)(void *);
        void (*GameUpdateFunction)(void *, float, void *);
        void (*GameShutdownFunction)(void *, void *);

        SDL_PathInfo _lastPathInfo;
        Uint64 _lastFileCheck;
    };

    static GameCodeObject GameCodeObjectInit(const char *_path)
    {
        GameCodeObject gco = {};
        gco.path = _path;

        // load gameplay shared library
        gco.sharedObjectHandle = SDL_LoadObject(_path);
        if (gco.sharedObjectHandle == NULL)
        {
            Debug::Log("Error loading shared object: %s", SDL_GetError());
            // return 1;
        }

        if (SDL_GetPathInfo(_path, &gco.pathInfo) == false)
        {
            Debug::Log("Error loading shared object path info: %s", SDL_GetError());
            // return 1;
        }

        gco._lastFileCheck = SDL_GetTicksNS();
        gco._lastPathInfo = gco.pathInfo;

        // load the exported function from mylib.so
        const char *gameInitName = "GameInit";
        gco.GameInitFunction = (void *(*)(void *))SDL_LoadFunction(gco.sharedObjectHandle, gameInitName);
        if (gco.GameInitFunction == NULL)
        {
            Debug::Log("Error loading function '%s': %s", gameInitName, SDL_GetError());
            SDL_UnloadObject(gco.sharedObjectHandle);
            // return 1;
        }

        const char *gameUpdateName = "GameUpdate";
        gco.GameUpdateFunction = (void (*)(void *, float, void *))SDL_LoadFunction(gco.sharedObjectHandle, gameUpdateName);
        if (gco.GameUpdateFunction == NULL)
        {
            Debug::Log("Error loading function '%s': %s", gameUpdateName, SDL_GetError());
            SDL_UnloadObject(gco.sharedObjectHandle);
            // return 1;
        }

        const char *gameShutdownName = "GameShutdown";
        gco.GameShutdownFunction = (void (*)(void *, void *))SDL_LoadFunction(gco.sharedObjectHandle, gameShutdownName);
        if (gco.GameShutdownFunction == NULL)
        {
            Debug::Log("Error loading function '%s': %s", gameShutdownName, SDL_GetError());
            SDL_UnloadObject(gco.sharedObjectHandle);
            // return 1;
        }

        return gco;
    }

    static void GameCodeObjectInitFunction(GameCodeObject *_gameCodeObject, Canis::App *_app)
    {
        _gameCodeObject->gameData = _gameCodeObject->GameInitFunction((void *)_app);
    }

    static void GameCodeObjectUpdateFunction(GameCodeObject *_gameCodeObject, Canis::App *_app, float _deltaTime)
    {
        _gameCodeObject->GameUpdateFunction((void *)_app, _deltaTime, _gameCodeObject->gameData);
    }

    static void GameCodeObjectWatchFile(GameCodeObject *_gameCodeObject, Canis::App *_app)
    {
        _gameCodeObject->GameShutdownFunction((void *)_app, _gameCodeObject->gameData);
        SDL_UnloadObject(_gameCodeObject->sharedObjectHandle);
        *_gameCodeObject = GameCodeObjectInit(_gameCodeObject->path);
        GameCodeObjectInitFunction(_gameCodeObject, _app);
        /*if (SDL_GetTicksNS() - _gameCodeObject->_lastFileCheck > 1.e9)
        {
            Debug::Log("Check");
            _gameCodeObject->_lastFileCheck = SDL_GetTicksNS();

            SDL_PathInfo info;
            if (SDL_GetPathInfo(_gameCodeObject->path, &info))
            {
                if (_gameCodeObject->pathInfo.modify_time < info.modify_time)
                {
                    if (_gameCodeObject->_lastPathInfo.size == info.size)
                    {
                        _gameCodeObject->GameShutdownFunction((void *)_app, _gameCodeObject->gameData);
                        SDL_UnloadObject(_gameCodeObject->sharedObjectHandle);
                        *_gameCodeObject = GameCodeObjectInit(_gameCodeObject->path);
                        GameCodeObjectInitFunction(_gameCodeObject, _app);
                    }
                }
            }

            _gameCodeObject->_lastPathInfo = info;
        }*/
    }
}