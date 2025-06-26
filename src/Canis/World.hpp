#pragma once
#include <vector>
#include "Camera.hpp"
#include "Entity.hpp"
#include "Window.hpp"
#include "InputManager.hpp"
#include "Data/PointLight.hpp"
#include "Data/DirectionalLight.hpp"

namespace Canis
{
    class World
    {
    public:
        World(Window *_window, InputManager *_inputManager, std::string _skyboxPath);
        void Update(double _deltaTime);
        void Draw(double _deltaTime);
        void Spawn(Entity _entity);
        void SpawnPointLight(PointLight _light);
        void SpawnDirectionalLight(DirectionalLight _light);
        Camera& GetCamera() { return m_camera; }
        Entity* GetEntity(unsigned int _index) { return &m_entities[_index]; }
        std::vector<Entity>& GetEntities() { return m_entities; }
        int GetEntitiesSize() { return m_entities.size(); }
        Entity* GetEntityWithTag(std::string _tag);
        std::vector<Entity*> GetEntitiesWithTag(std::string _tag);
        PointLight* GetPointLight(glm::vec3 _position); // returns nullptr when light is not found
        DirectionalLight& GetDirectionalLight() { return m_directionalLight; }

    private:
        InputManager *m_inputManager;
        Window *m_window;
        Camera m_camera = Camera(glm::vec3(0.0f, 0.0f, -3.0f));
        Shader m_skyboxShader;
        unsigned int m_skyboxId;
        Model m_skyboxModel;
        DirectionalLight m_directionalLight;
        std::vector<Entity> m_entities = {};
        std::vector<PointLight> m_pointLights = {};

        void UpdateLights(Canis::Shader &_shader);
        void UpdateCameraMovement(double _deltaTime);
    };
}