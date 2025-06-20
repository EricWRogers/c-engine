#include <vector>
#include <stdio.h>
#include <iostream>
#include <string>

extern "C" {
    #include <lua.h>
    #include <lualib.h>
    #include <lauxlib.h>
}

const int MAX_STRING = 512;
char stringBuilderBuffer[MAX_STRING];

void error(const char* _message)
{
    printf("error: %s\n", _message);
}

void printError(lua_State* state)
{
    error(lua_tostring(state, (int)-1));
    lua_pop(state, 1); // remove error message from top of stack
}

void RunScript(lua_State* state, const char* file)
{
    int result = luaL_loadfile(state, file);

    if (result == LUA_OK)
    {
        // use pcall to execute the script.
        result = lua_pcall(state, 0, LUA_MULTRET, 0);

        if (result != LUA_OK)
        {
            printError(state);
        }
    }
    else
    {
        printError(state);
    }
}

void Call(lua_State* state, const char* functionName)
{
    // pushes onto the stack the value of the global name of the lua function to be called.
    int type = lua_getglobal(state, functionName);

    if (type == LUA_TNIL)
    {
        snprintf(stringBuilderBuffer, sizeof(stringBuilderBuffer), "Attempted to call undefined Lua function: %s", functionName);
        error(stringBuilderBuffer);
    }
    else if (lua_pcall(state, 0, 0, 0) != 0)
    {
        printError(state);
    }
}

int Log(lua_State *_state)
{
    const char* text = lua_tostring(_state, (int)1);
    printf("%s\n", text);
    return 0;
}

int main() {
    lua_State* luaState = luaL_newstate();
    luaL_openlibs(luaState);

    lua_register(luaState, "Log", Log);

    RunScript(luaState, "assets/scripts/test.lua");

    Call(luaState, "Start");
    Call(luaState, "Update");

    lua_close(luaState);
}
