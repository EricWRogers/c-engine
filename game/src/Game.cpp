#include "../include/Game.hpp"

#include <stdio.h>
#include <stdlib.h>

#include <Canis/App.hpp>
#include <Canis/AssetManager.hpp>
#include <Canis/Debug.hpp>
#include <Canis/Entity.hpp>
#include <Canis/GameCodeObject.hpp>
#include <Canis/Time.hpp>
#include <Canis/Window.hpp>
#include <Canis/InputManager.hpp>

#include <Canis/ECS/Systems/SpriteRenderer2DSystem.hpp>

#include "../include/GameData.hpp"

using namespace Canis;

class BallMovement : public Canis::ScriptableEntity
{
public:
    Vector2 direction = Vector2(1.0f, 0.5f);
    float speed = 100.0f;
    float randomRotation = 0;

    RectTransform &transform = *entity.GetScript<RectTransform>();
    Sprite2D &sprite = *entity.GetScript<Sprite2D>();

    BallMovement(Canis::Entity &_entity) : Canis::ScriptableEntity(_entity) {}

    void Create()
    {
        direction = Vector2(1.0f, 0.5f).Normalize();
        direction = Vector2::Normalize(direction);
    }

    void Ready() { Canis::Debug::Log("OnReady"); }

    void Destroy() { Canis::Debug::Log("OnDestroy"); }

    void Update(float _dt)
    {
        Vector2 delta = direction * speed * Time::DeltaTime();
        transform.position += delta;
        transform.rotation += DEG2RAD * 100.0f * Time::DeltaTime();

        CheckWalls();
    }

    void EditorInspectorDraw() {
        std::string nameOfType = "BallMovement";
        ImGui::Text("%s", nameOfType.c_str());
        ImGui::InputFloat2("direction", &direction.x, "%.3f");
        ImGui::InputFloat("speed", &speed);
        ImGui::InputFloat("randomRotation", &randomRotation);
    }

    void CheckWalls()
    {
        Canis::Window &window = entity.scene->GetWindow();

        if (window.GetScreenWidth() * 0.5f <= transform.position.x + transform.size.x * 0.5f)
        {
            if (direction.x > 0.0f)
            {
                direction.x *= -1.0f;
            }
        }
        else if (-window.GetScreenWidth() * 0.5f >= transform.position.x - transform.size.x * 0.5f)
        {
            if (direction.x < 0.0f)
            {
                direction.x *= -1.0f;
            }
        }
        else if (window.GetScreenHeight() * 0.5f <= transform.position.y + transform.size.y * 0.5f)
        {
            if (direction.y > 0.0f)
            {
                direction.y *= -1.0f;
            }
        }
        else if (-window.GetScreenHeight() * 0.5f >= transform.position.y - transform.size.y * 0.5f)
        {
            if (direction.y < 0.0f)
            {
                direction.y *= -1.0f;
            }
        }
    }
};

ScriptConf ballMovementConf = {
    .name = "BallMovement",
    .Add = [](Entity& _entity) -> void { 
        // TODO: require a RectTransform component
        // TODO: require a Sprite2D component
        _entity.AddScript<BallMovement>();
     },
    .Has = [](Entity& _entity) -> bool { return (_entity.GetScript<BallMovement>() != nullptr); },
    .Remove = [](Entity& _entity) -> void { _entity.RemoveScript<BallMovement>(); },
    .Encode = [](YAML::Node &_node, Entity& _entity) -> void {
        if (_entity.GetScript<BallMovement>())
        {
            BallMovement& ball = *_entity.GetScript<BallMovement>();

            YAML::Node comp;

            comp["direction"] = ball.direction;
            comp["speed"] = ball.speed;
            comp["randomRotation"] = ball.randomRotation;

            _node[ballMovementConf.name] = comp;
        }
    },
    .Decode = [](YAML::Node &_node, Entity &_entity) -> void {
        Debug::Log("Check Add Ball");
        if (auto ballComponent = _node["BallMovement"])
        {
            Debug::Log("Add Ball");
            auto &ball = *_entity.AddScript<BallMovement>();
            ball.direction = ballComponent["direction"].as<Vector2>();
            ball.speed = ballComponent["speed"].as<float>();
            ball.randomRotation = ballComponent["randomRotation"].as<float>();
        }
    },
    .DrawInspector = [](Editor& _editor, Entity& _entity, const ScriptConf& _conf) -> void {
        BallMovement* ball = nullptr;
        if ((ball = _entity.GetScript<BallMovement>()) != nullptr)
        {
            ImGui::InputFloat2(("direction##" + _conf.name).c_str(), &ball->direction.x, "%.3f");
            ImGui::InputFloat(("speed##" + _conf.name).c_str(), &ball->speed);
            ImGui::InputFloat(("randomRotation##" + _conf.name).c_str(), &ball->randomRotation);
        }
    },
};



extern "C"
{
    void SpawnCamera(Canis::App &_app);
    void SpawnAwesome(Canis::App &_app);
    void ReloadScene(Canis::App &_app);

    InspectorItemRightClick inspectorCreateBall = {
        .name = "Create Ball",
        .Func = [](App& _app, Editor& _editor, Entity& _entity, std::vector<ScriptConf>& _scriptConfs) -> void {
            SpawnAwesome(_app);
        }
    };

    void *GameInit(void *_app)
    {
        Canis::App &app = *(Canis::App *)_app;
        
        app.RegisterInspectorItem(inspectorCreateBall);
        app.RegisterScript(ballMovementConf);

        Canis::Debug::Log("Game initialized!");
        GameData *gameData = (GameData *)malloc(sizeof(GameData));
        *gameData = GameData{};
        gameData->id = 15;
        return (void *)gameData;
    }

    void GameUpdate(void *_app, float dt, void *_data)
    {
        Canis::App &app = *(Canis::App *)_app;
        GameData &gameData = *(GameData *)_data;

        if (app.scene.GetInputManager().JustPressedKey(Canis::Key::C))
        {
            SpawnCamera(app);
        }

        // if (app.scene.GetInputManager().mouseRel != VECTOR2_ZERO)
        if (app.scene.GetInputManager().JustPressedKey(Canis::Key::B))
        {
            SpawnAwesome(app);
        }

        if (app.scene.GetInputManager().JustPressedKey(Canis::Key::SPACE))
        {
            ReloadScene(app);
        }

        //Canis::Debug::Log("Update FPS: %f", Canis::Time::FPS());
    }

    void GameShutdown(void *_app, void *_data)
    {
        Canis::App &app = *(Canis::App *)_app;

        app.UnregisterInspectorItem(inspectorCreateBall);
        app.UnregisterScript(ballMovementConf);

        Canis::Debug::Log("Game shutdown!");
        delete (GameData *)_data;
    }

    void SpawnCamera(Canis::App &_app)
    {
        Canis::Entity *cEntity = _app.scene.CreateEntity("Camera", "MainCamera");
        Canis::RectTransform * transform = cEntity->AddScript<Canis::RectTransform>();
        Canis::Camera2D *camera2D = cEntity->AddScript<Canis::Camera2D>();
    }

    void SpawnAwesome(Canis::App &_app)
    {
        Canis::Entity *entityOne = _app.scene.CreateEntity("Ball");
        Canis::RectTransform * transform = entityOne->AddScript<Canis::RectTransform>();
        Canis::Sprite2D *sprite = entityOne->AddScript<Canis::Sprite2D>();
        entityOne->AddScript<BallMovement>();

        sprite->textureHandle = Canis::AssetManager::GetTextureHandle("assets/textures/awesome_face.png");
        transform->size = Vector2(32.0f);
    }

    void ReloadScene(Canis::App &_app)
    {
        _app.scene.Unload();
        _app.scene.CreateRenderSystem<Canis::SpriteRenderer2DSystem>();
        _app.scene.Load(_app.GetScriptRegistry()); // call after all the systems are added

        SpawnAwesome(_app);
        SpawnCamera(_app);
        
    }
}
