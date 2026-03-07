#pragma once

#include <Canis/Entity.hpp>

namespace SpaceInvaders
{
    class Projectile : public Canis::ScriptableEntity
    {
    private:
        Canis::RectTransform* m_transform = nullptr;

    public:
        static constexpr const char* ScriptName = "SpaceInvaders::Projectile";

        Canis::Vector2 velocity = Canis::Vector2(0.0f, 400.0f);
        float lifeTime = 2.0f;
        bool fromPlayer = true;

        Projectile(Canis::Entity &_entity) : Canis::ScriptableEntity(_entity) {}

        void Create() override;
        void Ready() override;
        void Destroy() override;
        void Update(float _dt) override;
        void EditorInspectorDraw() override;
    };

    void RegisterProjectileScript(Canis::App& _app);
    void UnRegisterProjectileScript(Canis::App& _app);
}
