#pragma once

#include <Canis/Entity.hpp>

namespace SpaceInvaders
{
    class GameController : public Canis::ScriptableEntity
    {
    public:
        static constexpr const char* ScriptName = "SpaceInvaders::GameController";

        int score = 0;
        int lives = 3;
        bool gameOver = false;
        bool levelCleared = false;

        GameController(Canis::Entity &_entity) : Canis::ScriptableEntity(_entity) {}

        void Create() override;
        void Ready() override;
        void Destroy() override;
        void Update(float _dt) override;
        void EditorInspectorDraw() override;

        void AddScore(int _value);
        void OnPlayerHit();
        void SetGameOver(const char* _reason = nullptr);
    };

    void RegisterGameControllerScript(Canis::App& _app);
    void UnRegisterGameControllerScript(Canis::App& _app);
}
