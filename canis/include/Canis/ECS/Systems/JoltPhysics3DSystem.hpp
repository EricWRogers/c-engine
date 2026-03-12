#pragma once

#include <Canis/System.hpp>

namespace Canis
{
    class JoltPhysics3DSystem : public System
    {
    public:
        JoltPhysics3DSystem();
        ~JoltPhysics3DSystem();

        void Create() override;
        void Ready() override;
        void Update(entt::registry &_registry, float _deltaTime) override;
        void OnDestroy() override;

    private:
        struct Impl;
        Impl *m_impl = nullptr;
    };
} // namespace Canis
