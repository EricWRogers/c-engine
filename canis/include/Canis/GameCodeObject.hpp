#pragma once
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_loadso.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_filesystem.h>

struct GameCodeObject
{
    SDL_SharedObject *sharedObjectHandle;
    const char* path;
    SDL_PathInfo pathInfo;
    void *gameData;
    void *(*GameInitFunction)();
    void (*GameUpdateFunction)(float, void *);
    void (*GameShutdownFunction)(void *);
    
    SDL_PathInfo _lastPathInfo;
    Uint64 _lastFileCheck;
};

GameCodeObject GameCodeObjectInit(const char *_path)
{
    GameCodeObject gco = {};
    gco.path = _path;

    // load gameplay shared library
    gco.sharedObjectHandle = SDL_LoadObject("./libGameCode.so");
    if (gco.sharedObjectHandle == NULL)
    {
        SDL_Log("Error loading shared object: %s", SDL_GetError());
        // return 1;
    }

    if (SDL_GetPathInfo(_path, &gco.pathInfo) == false)
    {
        SDL_Log("Error loading shared object path info: %s", SDL_GetError());
        // return 1;
    }

    gco._lastFileCheck = SDL_GetTicksNS();
    gco._lastPathInfo = gco.pathInfo;

    // load the exported function from mylib.so
    const char *gameInitName = "GameInit";
    gco.GameInitFunction = (void *(*)())SDL_LoadFunction(gco.sharedObjectHandle, gameInitName);
    if (gco.GameInitFunction == NULL)
    {
        SDL_Log("Error loading function '%s': %s", gameInitName, SDL_GetError());
        SDL_UnloadObject(gco.sharedObjectHandle);
        // return 1;
    }

    const char *gameUpdateName = "GameUpdate";
    gco.GameUpdateFunction = (void (*)(float, void *))SDL_LoadFunction(gco.sharedObjectHandle, gameUpdateName);
    if (gco.GameUpdateFunction == NULL)
    {
        SDL_Log("Error loading function '%s': %s", gameUpdateName, SDL_GetError());
        SDL_UnloadObject(gco.sharedObjectHandle);
        // return 1;
    }

    const char *gameShutdownName = "GameShutdown";
    gco.GameShutdownFunction = (void (*)(void *))SDL_LoadFunction(gco.sharedObjectHandle, gameShutdownName);
    if (gco.GameShutdownFunction == NULL)
    {
        SDL_Log("Error loading function '%s': %s", gameShutdownName, SDL_GetError());
        SDL_UnloadObject(gco.sharedObjectHandle);
        // return 1;
    }

    return gco;
}

void GameCodeObjectInitFunction(GameCodeObject* _gameCodeObject)
{
    _gameCodeObject->gameData = _gameCodeObject->GameInitFunction();
}

void GameCodeObjectUpdateFunction(GameCodeObject* _gameCodeObject, float _deltaTime)
{
    _gameCodeObject->GameUpdateFunction(_deltaTime, _gameCodeObject->gameData);
}

void GameCodeObjectWatchFile(GameCodeObject* _gameCodeObject)
{
    if (SDL_GetTicksNS() - _gameCodeObject->_lastFileCheck > 1.e9)
    {
        SDL_Log("Check");
        _gameCodeObject->_lastFileCheck = SDL_GetTicksNS();

        SDL_PathInfo info;
        if (SDL_GetPathInfo(_gameCodeObject->path, &info))
        {
            if (_gameCodeObject->pathInfo.modify_time < info.modify_time)
            {
                if (_gameCodeObject->_lastPathInfo.size == info.size)
                {
                    _gameCodeObject->GameShutdownFunction(_gameCodeObject->gameData);
                    SDL_UnloadObject(_gameCodeObject->sharedObjectHandle);
                    *_gameCodeObject = GameCodeObjectInit(_gameCodeObject->path);
                    GameCodeObjectInitFunction(_gameCodeObject);
                }
            }
        }

        _gameCodeObject->_lastPathInfo = info;
    }
}
