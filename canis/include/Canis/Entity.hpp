#pragma once
#include <vector>
#include <functional>
#include <unordered_map>

#include <Canis/Math.hpp>
#include <Canis/AssetHandle.hpp>

#include <imgui.h>
#include <imgui_stdlib.h>

#include <Canis/Yaml.hpp>
#include <Canis/UUID.hpp>

namespace Canis
{
    class App;
    class Scene;
    class Editor;
    class ScriptableEntity;

    
    class Entity
    {
    friend Scene;
    friend Editor;
    private:
        std::vector<ScriptableEntity *> m_scriptComponents = {};
    public:
        int id;
        Scene *scene;
        bool active = true;
        std::string name = "";
        std::string tag = "";
        UUID uuid;
        
        Entity() = default;

        template <typename T>
        T *AddScript(bool _callCreate = true)
        {
            T *scriptableEntity = new T(*this);

            // might check if the entity already has script

            m_scriptComponents.push_back((ScriptableEntity*)scriptableEntity);

            if (_callCreate)
                scriptableEntity->Create();

            return scriptableEntity;
        }

        template <typename T>
        T *GetScript()
        {
            T *scriptableEntity = nullptr;

            for (ScriptableEntity *sc : m_scriptComponents)
            {
                if ((scriptableEntity = dynamic_cast<T *>(sc)) != nullptr)
                {
                    return scriptableEntity;
                }
            }

            return scriptableEntity;
        }

        template <typename T>
        void RemoveScript()
        {
            T *scriptableEntity = nullptr;

            for (int i = 0; i < m_scriptComponents.size(); i++)
            {
                if ((scriptableEntity = dynamic_cast<T *>(m_scriptComponents[i])) != nullptr)
                {
                    scriptableEntity->Destroy();
                    delete scriptableEntity;
                    m_scriptComponents.erase(m_scriptComponents.begin() + i);
                    return;
                }
            }
        }

        void Destroy();
    };

    class ScriptableEntity
    {
    friend Scene;
    private:
        bool m_onReadyCalled = false;
    public:        
        ScriptableEntity(Canis::Entity& _entity) : entity(_entity) {}

        Canis::Entity& entity;
        virtual void Create() {}
        virtual void Ready() {}
        virtual void Destroy() {}
        virtual void Update(float _dt) {}
        virtual void EditorInspectorDraw() {}
    };

    struct ScriptConf {
        std::string name;
        std::function<void(Entity&)> Add = nullptr;
        std::function<bool(Entity&)> Has = nullptr;
        std::function<void(Entity&)> Remove = nullptr;
        std::function<void(YAML::Node &_node, Entity &_entity)> Encode = nullptr;
        std::function<void(YAML::Node &_node, Entity &_entity)> Decode = nullptr;
        std::function<void(Editor&, Entity&, const ScriptConf&)> DrawInspector = nullptr;
        //std::unordered_map<std::string, std::function<void>> exposedFunctions;
    };

    struct InspectorItemRightClick {
        std::string name;
        std::function<void(App&, Editor&, Entity&, std::vector<ScriptConf>&)> Func = nullptr;
    };

    class RectTransform : public ScriptableEntity
    {
    public:
        RectTransform(Canis::Entity& _entity) : Canis::ScriptableEntity(_entity) {}

        void EditorInspectorDraw() {
            std::string nameOfType = "RectTransform";
            ImGui::Text("%s", nameOfType.c_str());
            ImGui::InputFloat2("position", &position.x, "%.3f");
            ImGui::InputFloat2("size", &size.x, "%.3f");
            ImGui::InputFloat2("scale", &scale.x);
            ImGui::InputFloat2("originOffset", &originOffset.x, "%.3f");
            ImGui::InputFloat("depth", &depth);
            // let user work with degrees
            float degrees = RAD2DEG * rotation;
            ImGui::InputFloat("rotation", &degrees);
            rotation = DEG2RAD * degrees;
        }

        bool active = true;
        Vector2 position = Vector2(0.0f);
        Vector2 size = Vector2(32.0f);
        Vector2 scale = Vector2(1.0f);
        Vector2 originOffset = Vector2(0.0f);
        float   depth = 0.0f;
        float   rotation = 0.0f;
        Vector2 rotationOriginOffset = Vector2(0.0f);
        Entity  parent;
		std::vector<Entity> children;
    };

    class Sprite2D : public ScriptableEntity
    {
    public:
        Sprite2D(Canis::Entity& _entity) : Canis::ScriptableEntity(_entity) {}

        void EditorInspectorDraw() {
            std::string nameOfType = "Sprite2D";
            ImGui::Text("%s", nameOfType.c_str());
            ImGui::ColorEdit4("color", &color.r);
            ImGui::InputFloat4("uv", &uv.x, "%.3f");
            // textureHandle
        }

        TextureHandle textureHandle;
        Color   color = Color(1.0f);
        Vector4 uv = Vector4(0.0f, 0.0f, 1.0f, 1.0f);
    };

    class Camera2D : public ScriptableEntity
    {
    public:
        Camera2D(Canis::Entity& _entity);
        ~Camera2D();

        void Create();

        void Update(float _dt);

        void SetPosition(const Vector2 &newPosition)
        {
            m_position = newPosition;
            UpdateMatrix();
        }
        void SetScale(float newScale)
        {
            m_scale = newScale;
            UpdateMatrix();
        }

        void EditorInspectorDraw() {
            std::string nameOfType = "Camera2D";
            ImGui::Text("%s", nameOfType.c_str());

            Vector2 lastPosition = GetPosition();
            float lastScale = GetScale();

            ImGui::InputFloat2("position", &lastPosition.x, "%.3f");
            ImGui::InputFloat("scale", &lastScale);

            if (lastPosition != GetPosition())
                SetPosition(lastPosition);
            
            if (lastScale != GetScale())
                SetScale(lastScale);
        }

        Vector2 GetPosition() { return m_position; }
        Matrix4 GetCameraMatrix() { return m_cameraMatrix; }
        Matrix4 GetViewMatrix() { return m_view; }
        Matrix4 GetProjectionMatrix() { return m_projection; }
        float GetScale() { return m_scale; }
        void UpdateMatrix();


    private:
        int m_screenWidth, m_screenHeight;
        bool m_needsMatrixUpdate;
        float m_scale = 1.0f;
        Vector2 m_position = Vector2(0.0f);
        Matrix4 m_cameraMatrix;
        Matrix4 m_view;
        Matrix4 m_projection;
    };
}
