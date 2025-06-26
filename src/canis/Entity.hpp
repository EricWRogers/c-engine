#pragma once
#include "Script.hpp"

namespace Canis
{
class Scene;

class Entity {
public:
    int id = -1;
    Scene* scene = nullptr;

    Entity() = default;
    Entity(int _id, Scene* _scene) {
        id = _id;
        scene = _scene;
    }

    void AddScript(std::string _className);
    void Destroy();
};
}