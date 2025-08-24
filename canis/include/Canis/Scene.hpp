#pragma once
#include <vector>

namespace Canis
{
    class App;
    class Entity;

    class Scene
    {
    public:
        App* app = nullptr;
        
        void Init(App *_app);
        void Update(float _deltaTime);
        void Unload();

        Entity* CreateEntity();
        Entity* GetEntity(int _id);

        void Destroy(int _id);
    private:
        std::vector <Entity*> m_entities = {};
    };
}
