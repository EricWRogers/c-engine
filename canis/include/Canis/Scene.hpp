#pragma once
#include <Canis/Entity.hpp>

namespace Canis
{
    class App;

    class Scene
    {
    public:
        App* app = nullptr;
        
        void Init(App *_app);
        void Update(float _deltaTime);
        void Unload();

        Entity* CreateEntity();
        void Destroy(int _id);
    private:
        std::vector <Entity*> m_entities = {};
    };
}
