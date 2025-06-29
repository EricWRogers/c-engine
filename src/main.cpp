#include <angelscript.h>
#include <scriptstdstring.h>
#include <scriptbuilder.h>
#include <scriptmath.h>
#include <cassert>

#include <iostream>
#include <SDL.h>
#include <glm/glm.hpp>
#include <string>
#include <fstream>
#include "Canis/Canis.hpp"
#include "Canis/Math.hpp"
#include "Canis/Graphics.hpp"
#include "Canis/Window.hpp"
#include "Canis/Shader.hpp"
#include "Canis/Debug.hpp"
#include "Canis/IOManager.hpp"
#include "Canis/InputManager.hpp"
#include "Canis/Camera.hpp"
#include "Canis/Model.hpp"
#include "Canis/World.hpp"
#include "Canis/Editor.hpp"
#include "Canis/FrameRateManager.hpp"
#include "Canis/ScriptInstance.hpp"

using namespace glm;

// git restore .
// git fetch
// git pull

void Print(const std::string &msg)
{
    std::cout << "[Script] " << msg << std::endl;
}

void MessageCallback(const asSMessageInfo *msg, void *)
{
    const char *type = "ERROR";
    if (msg->type == asMSGTYPE_WARNING)
        type = "WARNING";
    else if (msg->type == asMSGTYPE_INFORMATION)
        type = "INFO";

    std::cerr << msg->section << " (" << msg->row << ", " << msg->col << ") : "
              << type << " : " << msg->message << std::endl;
}

static asIScriptEngine *engine = nullptr;

int InitScriptingEngine()
{
    engine = asCreateScriptEngine();
    if (!engine)
    {
        std::cerr << "Failed to create AngelScript engine" << std::endl;
        return -1;
    }

    engine->SetMessageCallback(asFUNCTION(MessageCallback), nullptr, asCALL_CDECL);

    RegisterStdString(engine);

    engine->RegisterGlobalFunction("void Print(const string &in)", asFUNCTION(Print), asCALL_CDECL);
    return 0;
}

// frame rate manager

Canis::FrameRateManager frameRateManager;

void RegisterFrame(asIScriptEngine *engine)
{
    int r = engine->SetDefaultNamespace("Canis"); assert(r >= 0);

    // Reference type, no handles
    r = engine->RegisterObjectType("FrameRateManager", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);

    r = engine->RegisterObjectMethod("FrameRateManager", "double GetDeltaTime()", asMETHOD(Canis::FrameRateManager, GetDeltaTime), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("FrameRateManager", "double GetFPS()", asMETHOD(Canis::FrameRateManager, GetFPS), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("FrameRateManager", "double GetMaxFPS()", asMETHOD(Canis::FrameRateManager, GetMaxFPS), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("FrameRateManager", "void SetTargetFPS(double)", asMETHOD(Canis::FrameRateManager, SetTargetFPS), asCALL_THISCALL); assert(r >= 0);

    // Register the global instance (pointer)
    r = engine->RegisterGlobalProperty("FrameRateManager Frame", static_cast<Canis::FrameRateManager*>(&frameRateManager)); assert(r >= 0);

    // Reset to global namespace
    r = engine->SetDefaultNamespace(""); assert(r >= 0);
}

// window
Canis::Window window;

void RegisterWindow(asIScriptEngine *engine)
{
    int r = engine->SetDefaultNamespace("Canis"); assert(r >= 0);

    // Reference type, no handles
    r = engine->RegisterObjectType("WindowManager", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);

    r = engine->RegisterObjectMethod("WindowManager", "void SetName(string)", asMETHOD(Canis::Window, SetWindowName), asCALL_THISCALL); assert(r >= 0);

    // Register the global instance (pointer)
    r = engine->RegisterGlobalProperty("WindowManager Window", static_cast<Canis::Window*>(&window)); assert(r >= 0);

    // Reset to global namespace
    r = engine->SetDefaultNamespace(""); assert(r >= 0);
}

void RegisterTransform(asIScriptEngine* engine)
{
    int r = engine->SetDefaultNamespace("Canis"); assert(r >= 0);

    r = engine->RegisterObjectType("Transform", sizeof(Canis::Transform), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<Canis::Transform>()); assert(r >= 0);

    r = engine->RegisterObjectProperty("Transform", "vec3 position", asOFFSET(Canis::Transform, position)); assert(r >= 0);
    r = engine->RegisterObjectProperty("Transform", "vec3 rotation", asOFFSET(Canis::Transform, rotation)); assert(r >= 0);
    r = engine->RegisterObjectProperty("Transform", "vec3 scale", asOFFSET(Canis::Transform, scale)); assert(r >= 0);

    r = engine->SetDefaultNamespace(""); assert(r >= 0);
}


void RegisterVec3(asIScriptEngine* engine)
{
    int r = engine->RegisterObjectType("vec3", sizeof(glm::vec3), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<glm::vec3>()); assert(r >= 0);
    r = engine->RegisterObjectProperty("vec3", "float x", asOFFSET(glm::vec3, x)); assert(r >= 0);
    r = engine->RegisterObjectProperty("vec3", "float y", asOFFSET(glm::vec3, y)); assert(r >= 0);
    r = engine->RegisterObjectProperty("vec3", "float z", asOFFSET(glm::vec3, z)); assert(r >= 0);

    r = engine->RegisterObjectMethod("vec3", "string ToString() const", asFUNCTION(Canis::Math::Vec3ToString), asCALL_CDECL_OBJFIRST); assert(r >= 0);
}

void RegisterEntity(asIScriptEngine* engine) {
    int r = engine->SetDefaultNamespace("Canis"); assert(r >= 0);

    r = engine->RegisterObjectType("Entity", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
    r = engine->RegisterObjectProperty("Entity", "Transform transform", asOFFSET(Canis::Entity, transform)); assert(r >= 0);
    r = engine->RegisterObjectProperty("Entity", "vec3 color", asOFFSET(Canis::Entity, color)); assert(r >= 0);

    r = engine->SetDefaultNamespace(""); assert(r >= 0);
}

void RegisterWorld(asIScriptEngine* engine) {
    int r = engine->SetDefaultNamespace("Canis"); assert(r >= 0);

    r = engine->RegisterObjectType("World", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
    r = engine->RegisterObjectMethod("World", "Entity@ GetEntityWithName(const string &in)", asMETHOD(Canis::World, GetEntityWithName), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("World", "Entity@ GetEntityWithTag(const string &in)", asMETHOD(Canis::World, GetEntityWithTag), asCALL_THISCALL); assert(r >= 0);

    r = engine->SetDefaultNamespace(""); assert(r >= 0);
}

void RegisterMath(asIScriptEngine* engine) {
    int r = engine->SetDefaultNamespace("Canis::Math"); assert(r >= 0);

    r = engine->RegisterGlobalFunction("float RandomFloat(float, float)", asFUNCTION(Canis::Math::RandomFloat), asCALL_CDECL); assert(r >= 0);

    r = engine->SetDefaultNamespace(""); assert(r >= 0); // Reset namespace
}

// 3d array
std::vector<std::vector<std::vector<unsigned int>>> map = {};

// declaring functions
void SpawnLights(Canis::World &_world);
void LoadMap(std::string _path);

int main(int argc, char *argv[])
{
    Canis::Init();
    InitScriptingEngine();
    RegisterFrame(engine);
    RegisterWindow(engine);
    RegisterVec3(engine);
    RegisterTransform(engine);
    RegisterEntity(engine);
    RegisterWorld(engine);
    RegisterMath(engine);
    RegisterScriptMath(engine);


    Canis::Log("ENGINE");

    if (!engine)
        Canis::Log("NO ENGINE");

    Canis::ScriptInstance script(engine, "TestOne", "assets/scripts/TestOne.as");

    if (script.IsValid())
    {
        script.Call("Create");
        script.Call("Start");

        float dt = 0.016f;
        script.CallFloat("Update", dt);

        script.Call("Destroy");
    }

    Canis::ScriptInstance script2(engine, "TestTwo", "assets/scripts/TestTwo.as");

    if (script2.IsValid())
    {
        script2.Call("Create");
        script2.Call("Start");

        float dt = 0.016f;
        script2.CallFloat("Update", dt);

        script2.Call("Destroy");
    }

    Canis::InputManager inputManager;

    frameRateManager.Init(60);

    /// SETUP WINDOW
    window.MouseLock(true);

    unsigned int flags = 0;

    if (Canis::GetConfig().fullscreen)
        flags |= Canis::WindowFlags::FULLSCREEN;

    window.Create("Hello Graphics", Canis::GetConfig().width, Canis::GetConfig().heigth, flags);
    /// END OF WINDOW SETUP

    Canis::World world(&window, &inputManager, "assets/textures/lowpoly-skybox/");
    SpawnLights(world);

    Canis::Editor editor(&window, &world, &inputManager);

    Canis::Graphics::EnableAlphaChannel();
    Canis::Graphics::EnableDepthTest();

    /// SETUP SHADER
    Canis::Shader shader;
    shader.Compile("assets/shaders/hello_shader.vs", "assets/shaders/hello_shader.fs");
    shader.AddAttribute("aPosition");
    shader.Link();
    shader.Use();
    shader.SetInt("MATERIAL.diffuse", 0);
    shader.SetInt("MATERIAL.specular", 1);
    shader.SetFloat("MATERIAL.shininess", 64);
    shader.SetBool("WIND", false);
    shader.UnUse();

    Canis::Shader grassShader;
    grassShader.Compile("assets/shaders/hello_shader.vs", "assets/shaders/hello_shader.fs");
    grassShader.AddAttribute("aPosition");
    grassShader.Link();
    grassShader.Use();
    grassShader.SetInt("MATERIAL.diffuse", 0);
    grassShader.SetInt("MATERIAL.specular", 1);
    grassShader.SetFloat("MATERIAL.shininess", 64);
    grassShader.SetBool("WIND", true);
    grassShader.SetFloat("WINDEFFECT", 0.2);
    grassShader.UnUse();
    /// END OF SHADER

    /// Load Image
    Canis::GLTexture texture = Canis::LoadImageGL("assets/textures/glass.png", true);
    Canis::GLTexture grassTexture = Canis::LoadImageGL("assets/textures/grass.png", false);
    Canis::GLTexture textureSpecular = Canis::LoadImageGL("assets/textures/container2_specular.png", true);
    /// End of Image Loading

    /// Load Models
    Canis::Model cubeModel = Canis::LoadModel("assets/models/cube.obj");
    Canis::Model grassModel = Canis::LoadModel("assets/models/plants.obj");
    /// END OF LOADING MODEL

    // Load Map into 3d array
    LoadMap("assets/maps/level.map");

    int glassCount = 0;

    // Loop map and spawn objects
    for (int y = 0; y < map.size(); y++)
    {
        for (int x = 0; x < map[y].size(); x++)
        {
            for (int z = 0; z < map[y][x].size(); z++)
            {
                Canis::Entity entity;
                entity.active = true;

                switch (map[y][x][z])
                {
                case 1: // places a glass block
                    entity.name = "glass_" + std::to_string(glassCount++);
                    entity.tag = "glass";
                    entity.albedo = &texture;
                    entity.specular = &textureSpecular;
                    entity.model = &cubeModel;
                    entity.shader = &shader;
                    entity.transform.position = vec3(x + 0.0f, y + 0.0f, z + 0.0f);
                    entity.script = new Canis::ScriptInstance(engine, "TestOne", "assets/scripts/TestOne.as");
                    world.Spawn(entity);

                    
                    break;
                case 2: // places a glass block
                    entity.tag = "grass";
                    entity.albedo = &grassTexture;
                    entity.specular = &textureSpecular;
                    entity.model = &grassModel;
                    entity.shader = &grassShader;
                    entity.transform.position = vec3(x + 0.0f, y + 0.0f, z + 0.0f);
                    entity.script = new Canis::ScriptInstance(engine, "TestTwo", "assets/scripts/TestTwo.as");
                    world.Spawn(entity);
                    
                    
                    break;
                default:
                    break;
                }

                Canis::Entity* e = (Canis::Entity*)&(world.GetEntities()[world.GetEntitiesSize()-1]);

                if (e->script->IsValid())
                {
                    asIScriptFunction* setEntityFunc = e->script->GetObject()->GetObjectType()->GetMethodByDecl("void SetEntity(Canis::Entity@)");
                    if (setEntityFunc) {
                        asIScriptContext* ctx = engine->CreateContext();
                        ctx->Prepare(setEntityFunc);
                        ctx->SetObject(e->script->GetObject());
                        ctx->SetArgObject(0, e);
                        ctx->Execute();
                        ctx->Release();
                    }

                    asIScriptFunction* setWorldFunc = e->script->GetObject()->GetObjectType()->GetMethodByDecl("void SetWorld(Canis::World@)");
                    if (setWorldFunc) {
                        asIScriptContext* ctx = engine->CreateContext();
                        ctx->Prepare(setWorldFunc);
                        ctx->SetObject(e->script->GetObject());
                        ctx->SetArgObject(0, &world);
                        ctx->Execute();
                        ctx->Release();
                    }


                    e->script->Call("Create");
                }
                else
                {
                    e->script = nullptr;
                }
            }
        }
    }

    double deltaTime = 0.0;
    double fps = 0.0;

    // Application loop
    while (inputManager.Update(Canis::GetConfig().width, Canis::GetConfig().heigth))
    {
        deltaTime = frameRateManager.StartFrame();
        Canis::Graphics::ClearBuffer(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);

        world.Update(deltaTime);
        world.Draw(deltaTime);

        editor.Draw();

        window.SwapBuffer();

        // EndFrame will pause the app when running faster than frame limit
        fps = frameRateManager.EndFrame();
        // Canis::Log("FPS: " + std::to_string(fps) + " DeltaTime: " + std::to_string(deltaTime));
    }

    return 0;
}

void LoadMap(std::string _path)
{
    std::ifstream file;
    file.open(_path);

    if (!file.is_open())
    {
        printf("file not found at: %s \n", _path.c_str());
        exit(1);
    }

    int number = 0;
    int layer = 0;

    map.push_back(std::vector<std::vector<unsigned int>>());
    map[layer].push_back(std::vector<unsigned int>());

    while (file >> number)
    {
        if (number == -2) // add new layer
        {
            layer++;
            map.push_back(std::vector<std::vector<unsigned int>>());
            map[map.size() - 1].push_back(std::vector<unsigned int>());
            continue;
        }

        if (number == -1) // add new row
        {
            map[map.size() - 1].push_back(std::vector<unsigned int>());
            continue;
        }

        map[map.size() - 1][map[map.size() - 1].size() - 1].push_back((unsigned int)number);
    }
}

void SpawnLights(Canis::World &_world)
{
    Canis::DirectionalLight directionalLight;
    _world.SpawnDirectionalLight(directionalLight);

    Canis::PointLight pointLight;
    pointLight.position = vec3(0.0f);
    pointLight.ambient = vec3(0.2f);
    pointLight.diffuse = vec3(0.5f);
    pointLight.specular = vec3(1.0f);
    pointLight.constant = 1.0f;
    pointLight.linear = 0.09f;
    pointLight.quadratic = 0.032f;

    _world.SpawnPointLight(pointLight);

    pointLight.position = vec3(0.0f, 0.0f, 1.0f);
    pointLight.ambient = vec3(4.0f, 0.0f, 0.0f);

    _world.SpawnPointLight(pointLight);

    pointLight.position = vec3(-2.0f);
    pointLight.ambient = vec3(0.0f, 4.0f, 0.0f);

    _world.SpawnPointLight(pointLight);

    pointLight.position = vec3(2.0f);
    pointLight.ambient = vec3(0.0f, 0.0f, 4.0f);

    _world.SpawnPointLight(pointLight);
}
