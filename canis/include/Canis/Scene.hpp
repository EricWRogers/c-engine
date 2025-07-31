#pragma once
#include <Canis/Entity.hpp>

namespace Canis
{
    class Scene
    {
    public:
        Entity CreateEntity();
    private:
        std::vector <Entity*> m_entities = {};
    };
}