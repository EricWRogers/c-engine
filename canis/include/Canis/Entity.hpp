#pragma once
#include <map>
#include <vector>
#include <functional>
#include <unordered_map>
#include <algorithm>

#include <Canis/Math.hpp>
#include <Canis/AssetHandle.hpp>

#include <imgui.h>
#include <imgui_stdlib.h>

#include <Canis/Yaml.hpp>
#include <Canis/UUID.hpp>
#include <Canis/Debug.hpp>
#include <Canis/Data/Types.hpp>

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

    using PropertySetter = std::function<void(YAML::Node&, void*)>;
    using PropertyGetter = std::function<YAML::Node(void*)>;

    struct PropertyRegistry {
        std::map<std::string, PropertySetter> setters;
        std::map<std::string, PropertyGetter> getters;
        std::vector<std::string> propertyOrder;
    };

    struct ScriptConf {
        std::string name;
        PropertyRegistry registry;
        std::function<void(Entity&)> Add = nullptr;
        std::function<bool(Entity&)> Has = nullptr;
        std::function<void(Entity&)> Remove = nullptr;
        std::function<void*(Entity&)> Get = nullptr;
        std::function<void(YAML::Node &_node, Entity &_entity)> Encode = nullptr;
        std::function<void(YAML::Node &_node, Entity &_entity, bool _callCreate)> Decode = nullptr;
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
        float   depth = 0.001f;
        float   rotation = 0.0f;
        Vector2 rotationOriginOffset = Vector2(0.0f);
        Entity*  parent = nullptr;
		std::vector<Entity*> children;

        Vector2 GetPosition() const
        {
            Vector2 localPos = position + originOffset;

            if (!parent)
                return localPos;

            if (auto* parentRT = parent->GetScript<RectTransform>())
            {
                Vector2 parentPos = parentRT->GetPosition();
                float parentRot   = parentRT->GetRotation();
                Vector2 parentScale = parentRT->GetScale();

                Vector2 scaled(
                    localPos.x * parentScale.x,
                    localPos.y * parentScale.y
                );
                
                Vector2 rotatedLocal = RotatePoint(scaled, parentRot);
                return parentPos + rotatedLocal;
            }

            return localPos;
        }

        void SetPosition(Vector2 _globalPos)
        {
            if (parent)
            {
                if (auto* parentRT = parent->GetScript<RectTransform>())
                {
                    Vector2 parentPos = parentRT->GetPosition();
                    float parentRot   = parentRT->GetRotation();
                    
                    Vector2 parentSpace = _globalPos - parentPos;
                    Vector2 localPos = RotatePoint(parentSpace, -parentRot);

                    position = localPos - originOffset;
                    return;
                }
            }

            position = _globalPos - originOffset;
        }

        void Move(Vector2 _delta)
        {
            position = position + _delta;
        }

        float GetRotation() const
		{
            if (parent)
			{
                if (auto* parentRT = parent->GetScript<RectTransform>())
                {
                    return rotation + parentRT->GetRotation();
                }
			}

			return rotation;
		}

        float GetDepth() const
		{
            if (parent)
			{
                if (auto* parentRT = parent->GetScript<RectTransform>())
                {
                    return depth + parentRT->GetDepth();
                }
			}

			return depth;
		}

        void SetDepth(Vector2 _globalDepth)
        {
            if (parent)
            {
                if (auto* parentRT = parent->GetScript<RectTransform>())
                {
                    Vector2 parentDepth = parentRT->GetPosition();
                    
                    position = _globalDepth - parentDepth;
                    return;
                }
            }

            position = _globalDepth;
        }
    
        Vector2 GetScale() const
        {
            if (!parent)
                return scale;

            if (auto* parentRT = parent->GetScript<RectTransform>())
            {
                Vector2 p = parentRT->GetScale();
                return Vector2(p.x * scale.x, p.y * scale.y);
            }

            return scale;
        }

        void SetScale(const Vector2& _globalScale)
        {
            if (parent)
            {
                if (auto* parentRT = parent->GetScript<RectTransform>())
                {
                    Vector2 parentScale = parentRT->GetScale();

                    // avoid divide-by-zero
                    scale.x = (parentScale.x != 0.0f) ? (_globalScale.x / parentScale.x) : _globalScale.x;
                    scale.y = (parentScale.y != 0.0f) ? (_globalScale.y / parentScale.y) : _globalScale.y;
                    return;
                }
            }

            scale = _globalScale;
        }

        Vector2 GetRight() const
        {
            const float a = GetRotation();

            Vector2 right(std::cos(a), std::sin(a));

            return right.Normalize();
        }

        Vector2 GetUp() const
        {
            const float a = GetRotation();

            Vector2 up(-std::sin(a), std::cos(a));

            return up.Normalize();
        }

        bool HasParent() const {
            return parent != nullptr;
        }

        void SetParentAtIndex(Entity* newParent, std::size_t index)
        {
            Entity* self = &entity;;

            // same parent: just reorder within the same children list
            if (parent == newParent)
            {
                if (!parent)
                    return;

                if (auto* prt = parent->GetScript<RectTransform>())
                {
                    auto& list = prt->children;
                    auto it = std::find(list.begin(), list.end(), self);
                    if (it == list.end())
                        return;

                    std::size_t currentIndex = std::distance(list.begin(), it);
                    if (currentIndex == index)
                        return; // nothing to do

                    list.erase(it);

                    Clamp(index, 0, list.size());

                    list.insert(list.begin() + index, self);
                }
                return;
            }

            Vector2 oldPos = GetPosition();

            // different parent: remove from old parent
            if (parent)
            {
                if (auto* oldParentRT = parent->GetScript<RectTransform>())
                {
                    auto& list = oldParentRT->children;
                    list.erase(std::remove(list.begin(), list.end(), self), list.end());
                }
            }

            // set new parent pointer
            parent = newParent;

            // insert into new parent's children list at index
            if (newParent)
            {
                if (auto* newParentRT = newParent->GetScript<RectTransform>())
                {
                    auto& list = newParentRT->children;
                    Clamp(index, 0, list.size());

                    list.insert(list.begin() + index, self);
                }
            }

            SetPosition(oldPos);
        }

        void SetParent(Entity* newParent)
        {
            if (auto* rt = newParent ? newParent->GetScript<RectTransform>() : nullptr)
            {
                SetParentAtIndex(newParent, rt->children.size());
            }
            else
            {
                // unparent
                Unparent();
            }
        }

        void Unparent()
        {
            SetParentAtIndex(nullptr, 0);
        }

        bool IsChildOf(Entity* potentialParent) const
        {
            return parent == potentialParent;
        }

        bool HasChildren() const {
            return !children.empty();
        }

        void AddChild(Entity* child)
        {
            if (!child) return;

            auto* rt = child->GetScript<RectTransform>();
            if (!rt) return;

            rt->SetParent(&entity);
        }

        void RemoveChild(Entity* child)
        {
            if (!child) return;

            // remove from children list
            children.erase(std::remove(children.begin(), children.end(), child), children.end());

            // clear child's parent
            if (auto* rt = child->GetScript<RectTransform>())
                rt->parent = nullptr;
        }

        void RemoveAllChildren()
        {
            for (auto* child : children)
                if (auto* rt = child->GetScript<RectTransform>())
                    rt->parent = nullptr;
            
            children.clear();
        }

    };

    class Transform : public ScriptableEntity
    {
    public:
        Transform(Canis::Entity& _entity) : Canis::ScriptableEntity(_entity) {}

        void EditorInspectorDraw() {
            /*std::string nameOfType = "Transform";
            ImGui::Text("%s", nameOfType.c_str());
            ImGui::InputFloat3("position", &position.x, "%.3f");
            ImGui::InputFloat2("size", &size.x, "%.3f");
            ImGui::InputFloat2("scale", &scale.x);
            ImGui::InputFloat2("originOffset", &originOffset.x, "%.3f");
            ImGui::InputFloat("depth", &depth);
            // let user work with degrees
            float degrees = RAD2DEG * rotation;
            ImGui::InputFloat("rotation", &degrees);
            rotation = DEG2RAD * degrees;*/
        }

		bool active = true;
		Vector3 position = Vector3(0.0f);
		//glm::quat rotation = glm::quat(glm::vec3(0.0f, 0.0f, 0.0f));
        Vector3 rotation = Vector3(0.0f);
		Vector3 scale = Vector3(1.0f);
		Matrix4 modelMatrix = IdentitiyMatrix4();
		bool isDirty = true;
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

        void GetSpriteFromTextureAtlas(u8 _offsetX, u8 _offsetY, u16 &_indexX, u16 &_indexY, u16 _spriteWidth, u16 _spriteHeight, bool _flipX, bool _flipY)
        {
            uv.x = (_flipX) ? (((_indexX+1) * _spriteWidth) + _offsetX)/(f32)textureHandle.texture.width : (_indexX == 0) ? 0.0f : ((_indexX * _spriteWidth) + _offsetX)/(f32)textureHandle.texture.width;
            uv.y = (_flipY) ? (((_indexY+1) * _spriteHeight) + _offsetY)/(f32)textureHandle.texture.height : (_indexY == 0) ? 0.0f : ((_indexY * _spriteHeight) + _offsetY)/(f32)textureHandle.texture.height;
            uv.z = (_flipX) ? (_spriteWidth*-1.0f)/(float)textureHandle.texture.width : _spriteWidth/(float)textureHandle.texture.width;
            uv.w = (_flipY) ? (_spriteHeight*-1.0f)/(float)textureHandle.texture.height : _spriteHeight/(float)textureHandle.texture.height;
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
        void Destroy() {
            Debug::Log("DestroyCamera");
        }

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

    class SpriteAnimation : public ScriptableEntity
    {
    public:
        SpriteAnimation(Canis::Entity& _entity);
        ~SpriteAnimation();

        void Create() {}
        void Destroy() {}
        void Update(float _dt) {}
        void EditorInspectorDraw() {}

        void Play(std::string _path);

        void Pause()
        {
            speed = 0.0f;
        }

        unsigned short int animationId = 0u;
        float countDown = 0.0f;
        unsigned short int index = 0u;
        bool flipX = false;
        bool flipY = false;
        bool redraw = true;
        float speed = 1.0f;
    private:
    };
}
