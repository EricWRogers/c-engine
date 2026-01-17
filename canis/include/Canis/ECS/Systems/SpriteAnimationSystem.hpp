#pragma once
#include <Canis/System.hpp>

namespace Canis
{
    class SpriteAnimationSystem : public System
    {
    private:
    public:
        SpriteAnimationSystem() : System() { m_name = type_name<SpriteAnimationSystem>(); }

        void Create() {}

        void Ready() {}

        void Update();
    };
} // end of Canis namespace