#include "../../include/Pong/Paddle.hpp"

#include <Canis/App.hpp>
#include <Canis/Time.hpp>
#include <Canis/Scene.hpp>
#include <Canis/Window.hpp>
#include <Canis/Editor.hpp>
#include <Canis/InputManager.hpp>

#include "../../include/Pong/Ball.hpp"

using namespace Canis;

namespace Pong
{

ScriptConf paddleConf = {
    .name = "Pong::Paddle",
    .Add = [](Entity &_entity) -> void
    {
        // TODO: require a RectTransform component
        // TODO: require a Sprite2D component
        _entity.AddScript<Paddle>();
    },
    .Has = [](Entity &_entity) -> bool
    { return (_entity.GetScript<Paddle>() != nullptr); },
    .Remove = [](Entity &_entity) -> void
    { _entity.RemoveScript<Paddle>(); },
    .Encode = [](YAML::Node &_node, Entity &_entity) -> void
    {
        if (_entity.GetScript<Paddle>())
        {
            Paddle &paddle = *_entity.GetScript<Paddle>();

            YAML::Node comp;

            comp["direction"] = paddle.direction;
            comp["speed"] = paddle.speed;
            comp["playerNum"] = paddle.playerNum;
            comp["ball"] = paddle.ball->uuid;

            _node[paddleConf.name] = comp;
        }
    },
    .Decode = [](YAML::Node &_node, Entity &_entity, bool _callCreate) -> void
    {
        if (auto paddleComponent = _node[paddleConf.name])
        {
            auto &paddle = *_entity.AddScript<Paddle>(false);
            paddle.direction = paddleComponent["direction"].as<Vector2>(paddle.direction);
            paddle.speed = paddleComponent["speed"].as<float>(paddle.speed);
            paddle.playerNum = paddleComponent["playerNum"].as<int>(paddle.playerNum);
            _entity.scene->GetEntityAfterLoad(paddleComponent["ball"].as<Canis::UUID>(0),paddle.ball);
            if (_callCreate)
                paddle.Create();
        }
    },
    .DrawInspector = [](Editor &_editor, Entity &_entity, const ScriptConf &_conf) -> void
    {
        Paddle *paddle = nullptr;
        if ((paddle = _entity.GetScript<Paddle>()) != nullptr)
        {
            ImGui::InputFloat2(("direction##" + _conf.name).c_str(), &paddle->direction.x, "%.3f");
            ImGui::InputFloat(("speed##" + _conf.name).c_str(), &paddle->speed);
            ImGui::InputInt(("playerNum##" + _conf.name).c_str(), &paddle->playerNum, 0, 100);
            
            ImGui::Text("ball");
            ImGui::SameLine();

            std::string label;
            if (paddle->ball)
                label = "[ Entity: " + paddle->ball->name + " ]";
            else
                label = "[ missing entity ]";

            ImGui::Button(label.c_str(), ImVec2(150, 0));

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_DRAG"))
                {
                    const Canis::UUID dropped = *static_cast<const Canis::UUID*>(payload->Data);
                    Canis::Entity* entity = paddle->entity.scene->GetEntityWithUUID(dropped);

                    if (entity)
                        paddle->ball = entity;
                }
                ImGui::EndDragDropTarget();
            }

            if (ImGui::IsItemHovered() &&
                ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                if (paddle->ball)
                    _editor.FocusEntity(paddle->ball);
            }

            if (ImGui::BeginPopupContextItem("ball_ctx"))
            {
                if (ImGui::MenuItem("Clear"))
                    paddle->ball = nullptr;

                if (paddle->ball && ImGui::MenuItem("Select in Hierarchy"))
                    _editor.FocusEntity(paddle->ball);

                ImGui::EndPopup();
            }

        }
    },
};

void RegisterPaddleScript(Canis::App &_app)
{
    _app.RegisterScript(paddleConf);
}

void UnRegisterPaddleScript(Canis::App &_app)
{
    _app.UnregisterScript(paddleConf);
}

void Paddle::Create() {}
void Paddle::Ready() {}
void Paddle::Destroy() {}

void Paddle::Update(float _dt)
{
    InputManager& inputManager = entity.scene->GetInputManager();

    Canis::Clamp(playerNum, 1, 2);

    unsigned int up = (playerNum == 1) ? Canis::Key::W : Canis::Key::UP;
    unsigned int down = (playerNum == 1) ? Canis::Key::S : Canis::Key::DOWN;
    
    if (inputManager.GetKey(up))
        direction.y = 1.0f;
    else if (inputManager.GetKey(down))
        direction.y = -1.0f;
    else
        direction.y = 0.0f;
    
    Vector2 delta = direction * speed * Time::DeltaTime();
    transform.position += delta;

    if (Entity* e = entity.scene->FindEntityWithName("Ball"))
    {
        RectTransform& ballTransform = *e->GetScript<RectTransform>();

        float distance = transform.GetPosition().Distance2D(ballTransform.GetPosition());

        if (distance < (ballTransform.size.x * ballTransform.scale.x * 0.5f) + (transform.size.x * transform.scale.x * 0.5f))
        {
            Ball& ball = *e->GetScript<Ball>();

            ball.direction = (ballTransform.GetPosition() - transform.GetPosition()).Normalize();

        }
    }
}

void Paddle::EditorInspectorDraw()
{
    ImGui::Text("%s", paddleConf.name.c_str());
    ImGui::InputFloat2("direction", &direction.x, "%.3f");
    ImGui::InputFloat("speed", &speed);
}

} // namespace Pong