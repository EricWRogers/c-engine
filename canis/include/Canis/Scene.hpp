#pragma once
#include <Canis/Entity.hpp>

namespace Canis
{
    class Scene
    {
    
    public:
        
        void Init();
        void Update(float _deltaTime);
        void Unload();

        Entity* CreateEntity();
        void Destroy(int _id);
    private:
        std::vector <Entity*> m_entities = {};
    };
}
