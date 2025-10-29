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

        ScriptConf sprite2DConf = {
            .name = "Sprite2D",
            .Add = [this](Entity& _entity) -> void { _entity.AddScript<Sprite2D>(); },
            .Has = [this](Entity& _entity) -> bool { return (_entity.GetScript<Sprite2D>() != nullptr); },
            .Remove = [this](Entity& _entity) -> void { _entity.RemoveScript<Sprite2D>(); },
            .DrawInspector = [this](Editor& _editor, Entity& _entity, const ScriptConf& _conf) -> void {
                Sprite2D* sprite = nullptr;
                if ((sprite = _entity.GetScript<Sprite2D>()) != nullptr)
                {
                    ImGui::InputFloat2(("position##" + _conf.name).c_str(), &sprite->position.x, "%.3f");
                    ImGui::InputFloat2(("originOffset##" + _conf.name).c_str(), &sprite->originOffset.x, "%.3f");
                    ImGui::InputFloat(("depth##" + _conf.name).c_str(), &sprite->depth);
                    // let user work with degrees
                    ImGui::InputFloat(("rotation##" + _conf.name).c_str(), &sprite->rotation);
                    ImGui::InputFloat2(("size##" + _conf.name).c_str(), &sprite->size.x, "%.3f");
                    ImGui::ColorEdit4(("color##" + _conf.name).c_str(), &sprite->color.r);
                    ImGui::InputFloat4(("uv##" + _conf.name).c_str(), &sprite->uv.x, "%.3f");
                    // textureHandle
                }
            },
        };

        ScriptConf camera2DConf = {
            .name = "Camera2D",
            .Add = [this](Entity& _entity) -> void { _entity.AddScript<Camera2D>(); },
            .Has = [this](Entity& _entity) -> bool { return (_entity.GetScript<Camera2D>() != nullptr); },
            .Remove = [this](Entity& _entity) -> void { _entity.RemoveScript<Camera2D>(); },
            .DrawInspector = [this](Editor& _editor, Entity& _entity, const ScriptConf& _conf) -> void {
                Camera2D* camera = nullptr;
                if ((camera = _entity.GetScript<Camera2D>()) != nullptr)
                {
                    Vector2 lastPosition = camera->GetPosition();
                    float lastScale = camera->GetScale();

                    ImGui::InputFloat2(("position##" + _conf.name).c_str(), &lastPosition.x, "%.3f");
                    ImGui::InputFloat(("scale##" + _conf.name).c_str(), &lastScale);

                    if (lastPosition != camera->GetPosition())
                        camera->SetPosition(lastPosition);
                    
                    if (lastScale != camera->GetScale())
                        camera->SetScale(lastScale);
                }
            },
        };

        RegisterScript(sprite2DConf);
        RegisterScript(camera2DConf);

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
            editor.Draw(&scene, &window, this);
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

    void App::RegisterScript(ScriptConf &_conf)
    {
        for (ScriptConf &sc : m_scriptRegistry)
            if (_conf.name == sc.name)
                return;

        m_scriptRegistry.push_back(_conf);
    }

    void App::UnregisterScript(ScriptConf &_conf)
    {
        for (int i = 0; i < m_scriptRegistry.size(); i++)
        {
            if (_conf.name == m_scriptRegistry[i].name)
            {
                m_scriptRegistry.erase(m_scriptRegistry.begin() + i);
                i--;
            }
        }
    }

} // namespace Canis
