#pragma once
#include <vector>
#include <algorithm>
#include <stdexcept>

#include <Canis/Math.hpp>
#include <Canis/Asset.hpp>
#include <Canis/AssetHandle.hpp>
#include <Canis/UUID.hpp>
#include <Canis/Data/Types.hpp>
#include <Canis/Data/Bit.hpp>
#include <Canis/External/entt.hpp>
#include <Canis/Scene.hpp>

namespace Canis
{
    class App;
    class Scene;
    class Editor;
    class ScriptableEntity;
    struct ScriptConf;

    struct ScriptComponentEntry
    {
        std::string name = "";
        ScriptableEntity* script = nullptr;
    };

    
    class Entity
    {
    friend Scene;
    friend Editor;
    friend App;
    private:
        template <typename T>
        void InitializeComponent(T& _component)
        {
            if constexpr (requires { _component.entity; })
            {
                _component.entity = this;
            }
        }

        std::vector<ScriptComponentEntry> m_scriptComponents = {};
        entt::entity m_entityHandle = entt::null;

        ScriptableEntity* AddScriptDirect(const ScriptConf& _conf, ScriptableEntity* _scriptableEntity, bool _callCreate = true);
        ScriptableEntity* GetScriptDirect(const ScriptConf& _conf);
        const ScriptableEntity* GetScriptDirect(const ScriptConf& _conf) const;
        void RemoveScriptDirect(const ScriptConf& _conf);
    public:
        int id;
        Scene *scene;
        bool active = true;
        std::string name = "";
        std::string tag = "";
        UUID uuid;
        
        Entity() = default;

        entt::entity GetHandle() const
        {
            return m_entityHandle;
        }

        template <typename T, typename... Args>
        T& AddComponent(Args&&... _args)
        {
            if (!scene || m_entityHandle == entt::null)
                throw std::runtime_error("Entity::AddComponent called on invalid entity.");

            if (HasComponent<T>())
                return GetComponent<T>();

            T& component = scene->GetRegistry().emplace<T>(m_entityHandle, std::forward<Args>(_args)...);
            InitializeComponent(component);
            return component;
        }

        template <typename T, typename... Args>
        T& AddOrReplaceComponent(Args&&... _args)
        {
            if (!scene || m_entityHandle == entt::null)
                throw std::runtime_error("Entity::AddOrReplaceComponent called on invalid entity.");

            T& component = scene->GetRegistry().emplace_or_replace<T>(m_entityHandle, std::forward<Args>(_args)...);
            InitializeComponent(component);
            return component;
        }

        template <typename T>
        bool HasComponent() const
        {
            if (!scene || m_entityHandle == entt::null)
                return false;

            return scene->GetRegistry().all_of<T>(m_entityHandle);
        }

        template <typename T>
        T& GetComponent()
        {
            if (!scene || m_entityHandle == entt::null)
                throw std::runtime_error("Entity::GetComponent called on invalid entity.");

            return scene->GetRegistry().get<T>(m_entityHandle);
        }

        template <typename T>
        const T& GetComponent() const
        {
            if (!scene || m_entityHandle == entt::null)
                throw std::runtime_error("Entity::GetComponent called on invalid entity.");

            return scene->GetRegistry().get<T>(m_entityHandle);
        }

        template <typename T>
        void RemoveComponent()
        {
            if (!scene || m_entityHandle == entt::null)
                return;

            if (HasComponent<T>())
                scene->GetRegistry().remove<T>(m_entityHandle);
        }

        template <typename T>
        T* AddScript(bool _callCreate = true)
        {
            if (T* scriptableEntity = GetScript<T>())
                return scriptableEntity;

            T* scriptableEntity = new T(*this);

            m_scriptComponents.push_back(ScriptComponentEntry{
                .name = "",
                .script = scriptableEntity
            });

            if (_callCreate)
                scriptableEntity->Create();

            return scriptableEntity;
        }

        template <typename T>
        T* GetScript()
        {
            for (ScriptComponentEntry& entry : m_scriptComponents)
            {
                if (T* scriptableEntity = dynamic_cast<T*>(entry.script))
                    return scriptableEntity;
            }

            return nullptr;
        }

        template <typename T>
        const T* GetScript() const
        {
            for (const ScriptComponentEntry& entry : m_scriptComponents)
            {
                if (const T* scriptableEntity = dynamic_cast<const T*>(entry.script))
                    return scriptableEntity;
            }

            return nullptr;
        }

        template <typename T>
        void RemoveScript()
        {
            for (size_t i = 0; i < m_scriptComponents.size(); ++i)
            {
                if (T* scriptableEntity = dynamic_cast<T*>(m_scriptComponents[i].script))
                {
                    scriptableEntity->Destroy();
                    delete scriptableEntity;
                    m_scriptComponents.erase(m_scriptComponents.begin() + i);
                    return;
                }
            }
        }

        ScriptableEntity* AddScript(const std::string& _scriptName, bool _callCreate = true);
        ScriptableEntity* AttachScript(const std::string& _scriptName, ScriptableEntity* _scriptableEntity, bool _callCreate = true);
        ScriptableEntity* GetScript(const std::string& _scriptName);
        const ScriptableEntity* GetScript(const std::string& _scriptName) const;
        void RemoveScript(const std::string& _scriptName);
        bool HasScript(const std::string& _scriptName) const;
        void RemoveAllScripts();

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

    inline Entity* EntityPtr(Entity& _entity)
    {
        return &_entity;
    }

    inline Entity* EntityPtr(Entity* _entity)
    {
        return _entity;
    }

    template <typename TScript, typename TEntity>
    inline TScript* AddScriptTyped(TEntity&& _entity, bool _callCreate = true)
    {
        Entity* entity = EntityPtr(_entity);
        if (entity == nullptr)
            return nullptr;

        return static_cast<TScript*>(entity->AddScript(TScript::ScriptName, _callCreate));
    }

    template <typename TScript, typename TEntity>
    inline TScript* AttachScriptTyped(TEntity&& _entity, bool _callCreate = true)
    {
        Entity* entity = EntityPtr(_entity);
        if (entity == nullptr)
            return nullptr;

        return static_cast<TScript*>(entity->AttachScript(TScript::ScriptName, new TScript(*entity), _callCreate));
    }

    template <typename TScript, typename TEntity>
    inline TScript* GetScriptTyped(TEntity&& _entity)
    {
        Entity* entity = EntityPtr(_entity);
        if (entity == nullptr)
            return nullptr;

        return static_cast<TScript*>(entity->GetScript(TScript::ScriptName));
    }

    template <typename TScript, typename TEntity>
    inline void RemoveScriptTyped(TEntity&& _entity)
    {
        Entity* entity = EntityPtr(_entity);
        if (entity == nullptr)
            return;

        entity->RemoveScript(TScript::ScriptName);
    }

    template <typename TScript, typename TEntity>
    inline bool HasScriptTyped(TEntity&& _entity)
    {
        return GetScriptTyped<TScript>(_entity) != nullptr;
    }

    template <typename TComponent, typename TEntity>
    inline TComponent* AddComponentTyped(TEntity&& _entity)
    {
        Entity* entity = EntityPtr(_entity);
        if (entity == nullptr)
            return nullptr;

        return &entity->AddComponent<TComponent>();
    }

    template <typename TComponent, typename TEntity>
    inline TComponent* GetComponentTyped(TEntity&& _entity)
    {
        Entity* entity = EntityPtr(_entity);
        if (entity == nullptr || !entity->HasComponent<TComponent>())
            return nullptr;

        return &entity->GetComponent<TComponent>();
    }

    template <typename TComponent, typename TEntity>
    inline void RemoveComponentTyped(TEntity&& _entity)
    {
        Entity* entity = EntityPtr(_entity);
        if (entity == nullptr)
            return;

        entity->RemoveComponent<TComponent>();
    }

    template <typename TComponent, typename TEntity>
    inline bool HasComponentTyped(TEntity&& _entity)
    {
        return GetComponentTyped<TComponent>(_entity) != nullptr;
    }

#define CANIS_ADD_SCRIPT(entityExpr, type) \
    Canis::AddScriptTyped<type>((entityExpr), true)

#define CANIS_ADD_SCRIPT_WITH_CREATE(entityExpr, type, callCreate) \
    Canis::AddScriptTyped<type>((entityExpr), (callCreate))

#define CANIS_ATTACH_SCRIPT(entityExpr, type, callCreate) \
    Canis::AttachScriptTyped<type>((entityExpr), (callCreate))

#define CANIS_GET_SCRIPT(entityExpr, type) \
    Canis::GetScriptTyped<type>((entityExpr))

#define CANIS_REMOVE_SCRIPT(entityExpr, type) \
    Canis::RemoveScriptTyped<type>((entityExpr))

#define CANIS_HAS_SCRIPT(entityExpr, type) \
    Canis::HasScriptTyped<type>((entityExpr))

#define CANIS_ADD_COMPONENT(entityExpr, type) \
    Canis::AddComponentTyped<Canis::type>((entityExpr))

#define CANIS_GET_COMPONENT(entityExpr, type) \
    Canis::GetComponentTyped<Canis::type>((entityExpr))

#define CANIS_REMOVE_COMPONENT(entityExpr, type) \
    Canis::RemoveComponentTyped<Canis::type>((entityExpr))

#define CANIS_HAS_COMPONENT(entityExpr, type) \
    Canis::HasComponentTyped<Canis::type>((entityExpr))

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

    struct RectTransform
    {
    public:
        static constexpr const char* ScriptName = "Canis::RectTransform";

        RectTransform() = default;


        explicit RectTransform(Canis::Entity& _entity) : entity(&_entity) {}

        void EditorInspectorDraw();
        void Create() {}
        Entity* entity = nullptr;

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

            if (auto* parentRT = CANIS_GET_COMPONENT(parent, RectTransform))
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
                if (auto* parentRT = CANIS_GET_COMPONENT(parent, RectTransform))
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
                if (auto* parentRT = CANIS_GET_COMPONENT(parent, RectTransform))
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
                if (auto* parentRT = CANIS_GET_COMPONENT(parent, RectTransform))
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
                if (auto* parentRT = CANIS_GET_COMPONENT(parent, RectTransform))
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

            if (auto* parentRT = CANIS_GET_COMPONENT(parent, RectTransform))
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
                if (auto* parentRT = CANIS_GET_COMPONENT(parent, RectTransform))
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

            return glm::normalize(right);
        }

        Vector2 GetUp() const
        {
            const float a = GetRotation();

            Vector2 up(-std::sin(a), std::cos(a));

            return glm::normalize(up);
        }

        bool HasParent() const {
            return parent != nullptr;
        }

        void SetParentAtIndex(Entity* newParent, std::size_t index)
        {
            Entity* self = entity;
            if (self == nullptr)
                return;

            // same parent: just reorder within the same children list
            if (parent == newParent)
            {
                if (!parent)
                    return;

                if (auto* prt = CANIS_GET_COMPONENT(parent, RectTransform))
                {
                    auto& list = prt->children;
                    auto it = std::find(list.begin(), list.end(), self);
                    if (it == list.end())
                        return;

                    std::size_t currentIndex = std::distance(list.begin(), it);
                    if (currentIndex == index)
                        return; // nothing to do

                    list.erase(it);

                    index = std::clamp(index, static_cast<size_t>(0), list.size());

                    list.insert(list.begin() + index, self);
                }
                return;
            }

            Vector2 oldPos = GetPosition();

            // different parent: remove from old parent
            if (parent)
            {
                if (auto* oldParentRT = CANIS_GET_COMPONENT(parent, RectTransform))
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
                if (auto* newParentRT = CANIS_GET_COMPONENT(newParent, RectTransform))
                {
                    auto& list = newParentRT->children;
                    index = std::clamp(index, static_cast<size_t>(0), list.size());

                    list.insert(list.begin() + index, self);
                }
            }

            SetPosition(oldPos);
        }

        void SetParent(Entity* newParent)
        {
            if (auto* rt = newParent ? CANIS_GET_COMPONENT(newParent, RectTransform) : nullptr)
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

            auto* rt = CANIS_GET_COMPONENT(child, RectTransform);
            if (!rt) return;

            if (entity != nullptr)
                rt->SetParent(entity);
        }

        void RemoveChild(Entity* child)
        {
            if (!child) return;

            // remove from children list
            children.erase(std::remove(children.begin(), children.end(), child), children.end());

            // clear child's parent
            if (auto* rt = CANIS_GET_COMPONENT(child, RectTransform))
                rt->parent = nullptr;
        }

        void RemoveAllChildren()
        {
            for (auto* child : children)
                if (auto* rt = CANIS_GET_COMPONENT(child, RectTransform))
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

    struct Transform
    {
    public:
        static constexpr const char* ScriptName = "Canis::Transform";

        Transform() = default;


        explicit Transform(Canis::Entity& _entity) : entity(&_entity) {}
        Entity* entity = nullptr;
        void Create() {}

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
		Matrix4 modelMatrix = Matrix4(1.0f);
		bool isDirty = true;
        Entity  parent;
			std::vector<Entity> children;
    };

    struct Transform3D
    {
    public:
        static constexpr const char* ScriptName = "Canis::Transform3D";

        Transform3D() = default;


        explicit Transform3D(Canis::Entity& _entity) : entity(&_entity) {}
        Entity* entity = nullptr;

        void EditorInspectorDraw();
        void Create() {}
        void Destroy()
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
            Matrix4 matrix = Matrix4(1.0f);
            matrix = glm::translate(matrix, position);
            matrix = glm::rotate(matrix, rotation.z, Vector3(0.0f, 0.0f, 1.0f));
            matrix = glm::rotate(matrix, rotation.y, Vector3(0.0f, 1.0f, 0.0f));
            matrix = glm::rotate(matrix, rotation.x, Vector3(1.0f, 0.0f, 0.0f));
            matrix = glm::scale(matrix, scale);
            return matrix;
        }

        Matrix4 GetModelMatrix() const
        {
            Matrix4 local = GetLocalMatrix();

            if (parent != nullptr)
            {
                if (auto* parentTransform = CANIS_GET_COMPONENT(parent, Transform3D))
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
                if (auto* parentTransform = CANIS_GET_COMPONENT(parent, Transform3D))
                    return rotation + parentTransform->GetGlobalRotation();
            }

            return rotation;
        }

        Vector3 GetGlobalScale() const
        {
            if (parent != nullptr)
            {
                if (auto* parentTransform = CANIS_GET_COMPONENT(parent, Transform3D))
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
            return glm::normalize(Vector3(forward4.x, forward4.y, forward4.z));
        }

        Vector3 GetUp() const
        {
            const Matrix4 matrix = GetModelMatrix();
            Vector4 up4 = matrix * Vector4(0.0f, 1.0f, 0.0f, 0.0f);
            return glm::normalize(Vector3(up4.x, up4.y, up4.z));
        }

        bool HasParent() const
        {
            return parent != nullptr;
        }

        void SetParentAtIndex(Entity* newParent, std::size_t index)
        {
            Entity* self = entity;
            if (self == nullptr)
                return;

            if (parent == newParent)
            {
                if (!parent)
                    return;

                if (auto* parentTransform = CANIS_GET_COMPONENT(parent, Transform3D))
                {
                    auto& list = parentTransform->children;
                    auto it = std::find(list.begin(), list.end(), self);
                    if (it == list.end())
                        return;

                    std::size_t currentIndex = std::distance(list.begin(), it);
                    if (currentIndex == index)
                        return;

                    list.erase(it);
                    index = std::clamp(index, static_cast<size_t>(0), list.size());
                    list.insert(list.begin() + index, self);
                }
                return;
            }

            const Vector3 oldWorldPosition = GetGlobalPosition();
            const Vector3 oldWorldRotation = GetGlobalRotation();
            const Vector3 oldWorldScale = GetGlobalScale();

            if (parent)
            {
                if (auto* oldParentTransform = CANIS_GET_COMPONENT(parent, Transform3D))
                {
                    auto& list = oldParentTransform->children;
                    list.erase(std::remove(list.begin(), list.end(), self), list.end());
                }
            }

            parent = newParent;

            if (newParent)
            {
                if (auto* newParentTransform = CANIS_GET_COMPONENT(newParent, Transform3D))
                {
                    auto& list = newParentTransform->children;
                    index = std::clamp(index, static_cast<size_t>(0), list.size());
                    list.insert(list.begin() + index, self);

                    const Vector3 parentWorldPosition = newParentTransform->GetGlobalPosition();
                    const Vector3 parentWorldRotation = newParentTransform->GetGlobalRotation();
                    const Vector3 parentWorldScale = newParentTransform->GetGlobalScale();

                    Vector3 parentSpacePosition = oldWorldPosition - parentWorldPosition;
                    Matrix4 inverseParentRotation = Matrix4(1.0f);
                    inverseParentRotation = glm::rotate(inverseParentRotation, -parentWorldRotation.x, Vector3(1.0f, 0.0f, 0.0f));
                    inverseParentRotation = glm::rotate(inverseParentRotation, -parentWorldRotation.y, Vector3(0.0f, 1.0f, 0.0f));
                    inverseParentRotation = glm::rotate(inverseParentRotation, -parentWorldRotation.z, Vector3(0.0f, 0.0f, 1.0f));
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
            if (auto* transform = newParent ? CANIS_GET_COMPONENT(newParent, Transform3D) : nullptr)
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

            auto* transform = CANIS_GET_COMPONENT(child, Transform3D);
            if (!transform)
                return;

            if (entity != nullptr)
                transform->SetParent(entity);
        }

        void RemoveChild(Entity* child)
        {
            if (!child)
                return;

            children.erase(std::remove(children.begin(), children.end(), child), children.end());

            if (auto* transform = CANIS_GET_COMPONENT(child, Transform3D))
                transform->parent = nullptr;
        }

        void RemoveAllChildren()
        {
            for (auto* child : children)
            {
                if (auto* transform = child ? CANIS_GET_COMPONENT(child, Transform3D) : nullptr)
                    transform->parent = nullptr;
            }

            children.clear();
        }
    };

    struct Camera3D
    {
    public:
        static constexpr const char* ScriptName = "Canis::Camera3D";

        Camera3D() = default;


        explicit Camera3D(Canis::Entity& _entity) : entity(&_entity) {}
        Entity* entity = nullptr;
        void Create() {}

        void EditorInspectorDraw();

        bool primary = true;
        float fovDegrees = 60.0f;
        float nearClip = 0.1f;
        float farClip = 1000.0f;
    };

    struct DirectionalLight
    {
    public:
        static constexpr const char* ScriptName = "Canis::DirectionalLight";

        DirectionalLight() = default;


        explicit DirectionalLight(Canis::Entity& _entity) : entity(&_entity) {}
        Entity* entity = nullptr;
        void Create() {}

        void EditorInspectorDraw();

        bool enabled = true;
        Color color = Color(1.0f);
        float intensity = 1.0f;
        Vector3 direction = Vector3(-0.4f, -1.0f, -0.25f);
    };

    struct PointLight
    {
    public:
        static constexpr const char* ScriptName = "Canis::PointLight";

        PointLight() = default;


        explicit PointLight(Canis::Entity& _entity) : entity(&_entity) {}
        Entity* entity = nullptr;
        void Create() {}

        void EditorInspectorDraw();

        bool enabled = true;
        Color color = Color(1.0f);
        float intensity = 1.2f;
        float range = 12.0f;
    };

    struct Model3D
    {
    public:
        static constexpr const char* ScriptName = "Canis::Model3D";

        Model3D() = default;


        explicit Model3D(Canis::Entity& _entity) : entity(&_entity) {}
        Entity* entity = nullptr;
        void Create() {}

        void EditorInspectorDraw();

        i32 modelId = -1;
        Color color = Color(1.0f);
    };

    struct Material
    {
    public:
        static constexpr const char* ScriptName = "Canis::Material";

        Material() = default;


        explicit Material(Canis::Entity& _entity) : entity(&_entity) {}
        Entity* entity = nullptr;
        void Create() {}

        void EditorInspectorDraw();

        i32 materialId = -1;
        std::vector<i32> materialIds = {};
        Color color = Color(1.0f);
    };

    struct ModelAnimation3D
    {
    public:
        static constexpr const char* ScriptName = "Canis::ModelAnimation3D";

        ModelAnimation3D() = default;


        explicit ModelAnimation3D(Canis::Entity& _entity) : entity(&_entity) {}
        Entity* entity = nullptr;
        void Create() {}

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

    struct Sprite2D
    {
    public:
        static constexpr const char* ScriptName = "Canis::Sprite2D";

        Sprite2D() = default;


        explicit Sprite2D(Canis::Entity& _entity) : entity(&_entity) {}
        Entity* entity = nullptr;
        void Create() {}

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

    struct Text
    {
    public:
        static constexpr const char* ScriptName = "Canis::Text";

        Text() = default;
        explicit Text(Canis::Entity &_entity) : entity(&_entity) {}
        Entity* entity = nullptr;
        void Create() {}

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

    struct Camera2D
    {
    public:
        static constexpr const char* ScriptName = "Canis::Camera2D";

        Camera2D() = default;
        explicit Camera2D(Canis::Entity& _entity) : entity(&_entity) {}
        Entity* entity = nullptr;

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
        int m_screenWidth = 500;
        int m_screenHeight = 500;
        bool m_needsMatrixUpdate = true;
        float m_scale = 1.0f;
        Vector2 m_position = Vector2(0.0f);
        Matrix4 m_cameraMatrix = Matrix4(1.0f);
        Matrix4 m_view = Matrix4(1.0f);
        Matrix4 m_projection = Matrix4(1.0f);
    };

    struct SpriteAnimation
    {
    public:
        static constexpr const char* ScriptName = "Canis::SpriteAnimation";

        SpriteAnimation() = default;


        explicit SpriteAnimation(Canis::Entity& _entity) : entity(&_entity) {}
        Entity* entity = nullptr;
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
