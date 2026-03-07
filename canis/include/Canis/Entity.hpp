#pragma once
#include <vector>
#include <algorithm>

#include <Canis/Math.hpp>
#include <Canis/Asset.hpp>
#include <Canis/AssetHandle.hpp>
#include <Canis/UUID.hpp>
#include <Canis/Data/Types.hpp>
#include <Canis/Data/Bit.hpp>

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

    enum RectAnchor
	{
		TOPLEFT = 0,
		TOPCENTER = 1,
		TOPRIGHT = 2,
		CENTERLEFT = 3,
		CENTER = 4,
		CENTERRIGHT = 5,
		BOTTOMLEFT = 6,
		BOTTOMCENTER = 7,
		BOTTOMRIGHT = 8
	};

    static const char *RectAnchorLabels[] = {
		"Top Left", "Top Center", "Top Right",
		"Center Left", "Center", "Center Right",
		"Bottom Left", "Bottom Center", "Bottom Right"};

    class RectTransform : public ScriptableEntity
    {
    public:
        RectTransform(Canis::Entity& _entity) : Canis::ScriptableEntity(_entity) {}

        void EditorInspectorDraw();

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

        Vector2 static GetAnchor(const RectAnchor &_anchor, const float &_windowWidth, const float &_windowHeight)
        {
            switch (_anchor)
            {
            case RectAnchor::TOPLEFT:
            {
                return Vector2(0.0f, _windowHeight);
            }
            case RectAnchor::TOPCENTER:
            {
                return Vector2(_windowWidth / 2.0f, _windowHeight);
            }
            case RectAnchor::TOPRIGHT:
            {
                return Vector2(_windowWidth, _windowHeight);
            }
            case RectAnchor::CENTERLEFT:
            {
                return Vector2(0.0f, _windowHeight / 2.0f);
            }
            case RectAnchor::CENTER:
            {
                return Vector2(_windowWidth / 2.0f, _windowHeight / 2.0f);
            }
            case RectAnchor::CENTERRIGHT:
            {
                return Vector2(_windowWidth, _windowHeight / 2.0f);
            }
            case RectAnchor::BOTTOMLEFT:
            {
                return Vector2(0.0f, 0.0f);
            }
            case RectAnchor::BOTTOMCENTER:
            {
                return Vector2(_windowWidth / 2.0f, 0.0f);
            }
            case RectAnchor::BOTTOMRIGHT:
            {
                return Vector2(_windowWidth, 0.0f);
            }
            default:
            {
                return Vector2(0.0f);
            }
            }
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

    class Transform3D : public ScriptableEntity
    {
    public:
        Transform3D(Canis::Entity& _entity) : Canis::ScriptableEntity(_entity) {}

        void EditorInspectorDraw();
        void Destroy() override
        {
            Unparent();
            RemoveAllChildren();
        }

        bool active = true;
        Vector3 position = Vector3(0.0f);
        Vector3 rotation = Vector3(0.0f);
        Vector3 scale = Vector3(1.0f);
        Entity* parent = nullptr;
        std::vector<Entity*> children = {};

        Matrix4 GetLocalMatrix() const
        {
            Matrix4 matrix;
            matrix.Identity();
            matrix.Translate(position);
            matrix.Rotate(rotation.z, Vector3(0.0f, 0.0f, 1.0f));
            matrix.Rotate(rotation.y, Vector3(0.0f, 1.0f, 0.0f));
            matrix.Rotate(rotation.x, Vector3(1.0f, 0.0f, 0.0f));
            matrix.Scale(scale);
            return matrix;
        }

        Matrix4 GetModelMatrix() const
        {
            Matrix4 local = GetLocalMatrix();

            if (parent != nullptr)
            {
                if (auto* parentTransform = parent->GetScript<Transform3D>())
                    return parentTransform->GetModelMatrix() * local;
            }

            return local;
        }

        Vector3 GetGlobalPosition() const
        {
            const Matrix4 model = GetModelMatrix();
            const Vector4 world = model * Vector4(0.0f, 0.0f, 0.0f, 1.0f);
            return Vector3(world.x, world.y, world.z);
        }

        Vector3 GetGlobalRotation() const
        {
            if (parent != nullptr)
            {
                if (auto* parentTransform = parent->GetScript<Transform3D>())
                    return rotation + parentTransform->GetGlobalRotation();
            }

            return rotation;
        }

        Vector3 GetGlobalScale() const
        {
            if (parent != nullptr)
            {
                if (auto* parentTransform = parent->GetScript<Transform3D>())
                {
                    const Vector3 parentScale = parentTransform->GetGlobalScale();
                    return Vector3(
                        parentScale.x * scale.x,
                        parentScale.y * scale.y,
                        parentScale.z * scale.z);
                }
            }

            return scale;
        }

        Vector3 GetForward() const
        {
            const Matrix4 matrix = GetModelMatrix();
            Vector4 forward4 = matrix * Vector4(0.0f, 0.0f, -1.0f, 0.0f);
            return Normalize(Vector3(forward4.x, forward4.y, forward4.z));
        }

        Vector3 GetUp() const
        {
            const Matrix4 matrix = GetModelMatrix();
            Vector4 up4 = matrix * Vector4(0.0f, 1.0f, 0.0f, 0.0f);
            return Normalize(Vector3(up4.x, up4.y, up4.z));
        }

        bool HasParent() const
        {
            return parent != nullptr;
        }

        void SetParentAtIndex(Entity* newParent, std::size_t index)
        {
            Entity* self = &entity;

            if (parent == newParent)
            {
                if (!parent)
                    return;

                if (auto* parentTransform = parent->GetScript<Transform3D>())
                {
                    auto& list = parentTransform->children;
                    auto it = std::find(list.begin(), list.end(), self);
                    if (it == list.end())
                        return;

                    std::size_t currentIndex = std::distance(list.begin(), it);
                    if (currentIndex == index)
                        return;

                    list.erase(it);
                    Clamp(index, 0, list.size());
                    list.insert(list.begin() + index, self);
                }
                return;
            }

            const Vector3 oldWorldPosition = GetGlobalPosition();
            const Vector3 oldWorldRotation = GetGlobalRotation();
            const Vector3 oldWorldScale = GetGlobalScale();

            if (parent)
            {
                if (auto* oldParentTransform = parent->GetScript<Transform3D>())
                {
                    auto& list = oldParentTransform->children;
                    list.erase(std::remove(list.begin(), list.end(), self), list.end());
                }
            }

            parent = newParent;

            if (newParent)
            {
                if (auto* newParentTransform = newParent->GetScript<Transform3D>())
                {
                    auto& list = newParentTransform->children;
                    Clamp(index, 0, list.size());
                    list.insert(list.begin() + index, self);

                    const Vector3 parentWorldPosition = newParentTransform->GetGlobalPosition();
                    const Vector3 parentWorldRotation = newParentTransform->GetGlobalRotation();
                    const Vector3 parentWorldScale = newParentTransform->GetGlobalScale();

                    Vector3 parentSpacePosition = oldWorldPosition - parentWorldPosition;
                    Matrix4 inverseParentRotation;
                    inverseParentRotation.Identity();
                    inverseParentRotation.Rotate(-parentWorldRotation.x, Vector3(1.0f, 0.0f, 0.0f));
                    inverseParentRotation.Rotate(-parentWorldRotation.y, Vector3(0.0f, 1.0f, 0.0f));
                    inverseParentRotation.Rotate(-parentWorldRotation.z, Vector3(0.0f, 0.0f, 1.0f));
                    Vector4 localPosition4 = inverseParentRotation * Vector4(
                        parentSpacePosition.x,
                        parentSpacePosition.y,
                        parentSpacePosition.z,
                        0.0f);

                    position.x = (parentWorldScale.x != 0.0f) ? (localPosition4.x / parentWorldScale.x) : localPosition4.x;
                    position.y = (parentWorldScale.y != 0.0f) ? (localPosition4.y / parentWorldScale.y) : localPosition4.y;
                    position.z = (parentWorldScale.z != 0.0f) ? (localPosition4.z / parentWorldScale.z) : localPosition4.z;
                    rotation = oldWorldRotation - parentWorldRotation;
                    scale.x = (parentWorldScale.x != 0.0f) ? (oldWorldScale.x / parentWorldScale.x) : oldWorldScale.x;
                    scale.y = (parentWorldScale.y != 0.0f) ? (oldWorldScale.y / parentWorldScale.y) : oldWorldScale.y;
                    scale.z = (parentWorldScale.z != 0.0f) ? (oldWorldScale.z / parentWorldScale.z) : oldWorldScale.z;
                    return;
                }
            }

            position = oldWorldPosition;
            rotation = oldWorldRotation;
            scale = oldWorldScale;
        }

        void SetParent(Entity* newParent)
        {
            if (auto* transform = newParent ? newParent->GetScript<Transform3D>() : nullptr)
            {
                SetParentAtIndex(newParent, transform->children.size());
            }
            else
            {
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

        bool HasChildren() const
        {
            return !children.empty();
        }

        void AddChild(Entity* child)
        {
            if (!child)
                return;

            auto* transform = child->GetScript<Transform3D>();
            if (!transform)
                return;

            transform->SetParent(&entity);
        }

        void RemoveChild(Entity* child)
        {
            if (!child)
                return;

            children.erase(std::remove(children.begin(), children.end(), child), children.end());

            if (auto* transform = child->GetScript<Transform3D>())
                transform->parent = nullptr;
        }

        void RemoveAllChildren()
        {
            for (auto* child : children)
            {
                if (auto* transform = child ? child->GetScript<Transform3D>() : nullptr)
                    transform->parent = nullptr;
            }

            children.clear();
        }
    };

    class Camera3D : public ScriptableEntity
    {
    public:
        Camera3D(Canis::Entity& _entity) : Canis::ScriptableEntity(_entity) {}

        void EditorInspectorDraw();

        bool primary = true;
        float fovDegrees = 60.0f;
        float nearClip = 0.1f;
        float farClip = 1000.0f;
    };

    class Model3D : public ScriptableEntity
    {
    public:
        Model3D(Canis::Entity& _entity) : Canis::ScriptableEntity(_entity) {}

        void EditorInspectorDraw();

        i32 modelId = -1;
        Color color = Color(1.0f);
    };

    class ModelAnimation3D : public ScriptableEntity
    {
    public:
        ModelAnimation3D(Canis::Entity& _entity) : Canis::ScriptableEntity(_entity) {}

        void EditorInspectorDraw();

        bool playAnimation = true;
        bool loop = true;
        float animationSpeed = 1.0f;
        float animationTime = 0.0f;
        i32 animationIndex = 0;

        // Runtime pose cache for this entity's model instance.
        i32 poseModelId = -1;
        ModelAsset::Pose3D pose = {};

        // Runtime caching to skip redundant animation evaluation.
        bool poseInitialized = false;
        i32 lastEvaluatedAnimationIndex = -1;
        float lastEvaluatedAnimationTime = 0.0f;
    };

    class Sprite2D : public ScriptableEntity
    {
    public:
        Sprite2D(Canis::Entity& _entity) : Canis::ScriptableEntity(_entity) {}

        void EditorInspectorDraw();

        void GetSpriteFromTextureAtlas(u8 _offsetX, u8 _offsetY, u16 _indexX, u16 _indexY, u16 _spriteWidth, u16 _spriteHeight)
        {
            uv.x = (flipX) ? (((_indexX+1) * _spriteWidth) + _offsetX)/(f32)textureHandle.texture.width : (_indexX == 0) ? 0.0f : ((_indexX * _spriteWidth) + _offsetX)/(f32)textureHandle.texture.width;
            uv.y = (flipY) ? (((_indexY+1) * _spriteHeight) + _offsetY)/(f32)textureHandle.texture.height : (_indexY == 0) ? 0.0f : ((_indexY * _spriteHeight) + _offsetY)/(f32)textureHandle.texture.height;
            uv.z = (flipX) ? (_spriteWidth*-1.0f)/(float)textureHandle.texture.width : _spriteWidth/(float)textureHandle.texture.width;
            uv.w = (flipY) ? (_spriteHeight*-1.0f)/(float)textureHandle.texture.height : _spriteHeight/(float)textureHandle.texture.height;
        }

        TextureHandle textureHandle;
        Color   color = Color(1.0f);
        Vector4 uv = Vector4(0.0f, 0.0f, 1.0f, 1.0f);
        bool flipX = false;
        bool flipY = false;
    };

    namespace TextAlignment
    {
        constexpr unsigned int LEFT = 0u;
        constexpr unsigned int RIGHT = 1u;
        constexpr unsigned int CENTER = 2u;
    }

    namespace TextBoundary
    {
        constexpr unsigned int OVERFLOW = 0u;
        constexpr unsigned int WRAP = 1u;
    }

    class Text : public ScriptableEntity
    {
    public:
        Text(Canis::Entity &_entity) : Canis::ScriptableEntity(_entity) {}

        void EditorInspectorDraw();

        void SetText(const std::string &_text)
        {
            text = _text;
            _status |= BIT::ONE;
        }

        i32 assetId = -1;
        std::string text = "";
        Color color = Color(1.0f);
        unsigned int alignment = TextAlignment::LEFT;
        unsigned int horizontalBoundary = TextBoundary::OVERFLOW;
        unsigned int _status = BIT::ONE;
    };

    class Camera2D : public ScriptableEntity
    {
    public:
        Camera2D(Canis::Entity& _entity);
        ~Camera2D();

        void Create();
        void Destroy();

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

        void EditorInspectorDraw();

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
        SpriteAnimation(Canis::Entity& _entity) : Canis::ScriptableEntity(_entity) {}
        ~SpriteAnimation() {}

        void Create() {}
        void Destroy() {}
        void Update(float _dt) {}
        void EditorInspectorDraw() {}

        void Play(std::string _path);

        void Pause()
        {
            speed = 0.0f;
        }

        i32 id = 0u;
        f32 speed = 1.0f;

        // hidden in editor
        f32 countDown = 0.0f;
        u16 index = 0u;
        bool redraw = true;
    private:
    };
}
