#include <Canis/Scene.hpp>
#include <Canis/App.hpp>
#include <Canis/Yaml.hpp>
#include <Canis/Editor.hpp>
#include <Canis/Debug.hpp>
#include <Canis/Entity.hpp>
#include <Canis/AssetManager.hpp>
#include <Canis/System.hpp>
#include <Canis/Window.hpp>
#include <Canis/ECS/Systems/SpriteAnimationSystem.hpp>
#include <Canis/ECS/Systems/SpriteRenderer2DSystem.hpp>
#include <Canis/ECS/Systems/ModelAnimation3DSystem.hpp>
#include <Canis/ECS/Systems/MeshRenderer3DSystem.hpp>
#include <Canis/ECS/Systems/JoltPhysics3DSystem.hpp>
#include <algorithm>
#include <cctype>
#include <SDL3/SDL_timer.h>

namespace Canis
{
    void Scene::Init(App *_app, Window *_window, InputManager *_inputManger, std::string _path)
    {
        app = _app;
        m_window = _window;
        m_inputManager = _inputManger;
        m_path = _path;
        ClearEditorCameraOverrides();

        // TODO resizing breaks components
        // I think this is fixed but needs more testing
        m_entities.reserve(1000);
    }

    void Scene::SetEditorCamera3DOverride(const Matrix4 &_view, const Matrix4 &_projection)
    {
        m_editorCamera3DOverrideEnabled = true;
        m_editorCamera3DView = _view;
        m_editorCamera3DProjection = _projection;
    }

    void Scene::SetEditorCamera2DOverride(const Matrix4 &_cameraMatrix, const Vector2 &_position)
    {
        m_editorCamera2DOverrideEnabled = true;
        m_editorCamera2DMatrix = _cameraMatrix;
        m_editorCamera2DPosition = _position;
    }

    void Scene::ClearEditorCameraOverrides()
    {
        m_editorCamera3DOverrideEnabled = false;
        m_editorCamera2DOverrideEnabled = false;
    }

    void Scene::Update(float _deltaTime)
    {
        m_isUpdating = true;

        for (System* system : m_updateSystems)
        {
            const Uint64 start = SDL_GetTicksNS();
            system->Update(m_registry, _deltaTime);
            const float elapsedMs = static_cast<float>(SDL_GetTicksNS() - start) / 1000000.0f;
            if (SystemTiming* timing = GetSystemTiming(system))
                timing->updateMs = elapsedMs;
        }
        // Run Ready only on entities created since the last frame.
        for (size_t i = 0; i < m_entitiesToReady.size(); ++i)
        {
            const int id = m_entitiesToReady[i];
            if (id < 0 || id >= (int)m_entities.size())
                continue;

            Entity* e = m_entities[id];
            if (e == nullptr || !e->active)
                continue;

            auto& scripts = e->m_scriptComponents;
            for (size_t j = 0; j < scripts.size() && e->active; ++j)
            {
                ScriptableEntity* se = scripts[j];
                if (!se || se->m_onReadyCalled)
                    continue;

                se->Ready();
                se->m_onReadyCalled = true;
            }
        }
        m_entitiesToReady.clear();

        for (size_t i = 0; i < m_entities.size(); ++i)
        {
            Entity* e = m_entities[i];
            if (e == nullptr || !e->active)
                continue;

            auto& scripts = e->m_scriptComponents;
            for (size_t j = 0; j < scripts.size() && e->active; ++j)
            {
                ScriptableEntity* se = scripts[j];
                if (se && se->m_onReadyCalled)
                    se->Update(_deltaTime);
            }
        }

        m_isUpdating = false;
        for (int id : m_entitiesToDestroy)
        {
            DestroyNow(id);
        }
        m_entitiesToDestroy.clear();
    }

    void Scene::Render(float _deltaTime)
    {
        //Canis::Debug::Log("Render Update %i", m_renderSystems.size());
        for (System* renderer : m_renderSystems)
        {
            const Uint64 start = SDL_GetTicksNS();
            renderer->Update(m_registry, _deltaTime);
            const float elapsedMs = static_cast<float>(SDL_GetTicksNS() - start) / 1000000.0f;
            if (SystemTiming* timing = GetSystemTiming(renderer))
                timing->renderMs = elapsedMs;
        }
    }

    void Scene::Unload()
    {
        for (Entity* e : m_entities)
        {
            if (e == nullptr)
                continue;
            e->RemoveAllScripts();
            delete e;
        }

        m_entities.clear();
        m_registry.clear();

        for (System* system : m_systems)
        {
            system->OnDestroy();
        }

        for (System* system : m_systems)
        {
            delete system;
        }

        m_systems.clear();
        m_updateSystems.clear();
        m_renderSystems.clear();
        m_systemTimings.clear();
    }

    void Scene::Load(std::vector<ScriptConf>& _scriptRegistry)
    {
        YAML::Node root = YAML::LoadFile(m_path);

        if (!root)
        {
            Debug::FatalError("Scene not found: %s", m_path.c_str());
        }
        
        LoadSceneNode(_scriptRegistry, root);
    }

    void Scene::LoadSceneNode(std::vector<ScriptConf>& _scriptRegistry, YAML::Node &_root)
    {
        CreateRenderSystem<Canis::MeshRenderer3DSystem>();
        CreateRenderSystem<Canis::SpriteRenderer2DSystem>();
        CreateSystem<Canis::ModelAnimation3DSystem>();
        CreateSystem<Canis::SpriteAnimationSystem>();
        CreateSystem<Canis::JoltPhysics3DSystem>();
        m_environmentSkyboxUUID = UUID(0);
        
        for (System* system : m_systems)
        {
            system->Create();
        }

        for (System* system : m_systems)
        {
            system->Ready();
        }

        auto environment = _root["Environment"]; 

        if (environment)
        {
            m_window->SetClearColor(
                environment["ClearColor"].as<Vector4>(Vector4(0.05f, 0.05f, 0.05f, 1.0f))
            );

            if (YAML::Node skyboxNode = environment["SkyboxAsset"])
            {
                if (skyboxNode.IsMap())
                {
                    m_environmentSkyboxUUID = skyboxNode["uuid"].as<uint64_t>(0);
                    if ((uint64_t)m_environmentSkyboxUUID == 0)
                    {
                        const std::string path = skyboxNode["path"].as<std::string>("");
                        if (!path.empty())
                        {
                            if (MetaFileAsset *meta = AssetManager::GetMetaFile(path))
                                m_environmentSkyboxUUID = meta->uuid;
                        }
                    }
                }
                else if (skyboxNode.IsScalar())
                {
                    const std::string rawValue = skyboxNode.as<std::string>("");
                    const bool isNumeric = std::all_of(rawValue.begin(), rawValue.end(), [](unsigned char c)
                        { return std::isdigit(c) != 0; });

                    if (isNumeric && !rawValue.empty())
                    {
                        m_environmentSkyboxUUID = (UUID)std::stoull(rawValue);
                    }
                    else if (!rawValue.empty())
                    {
                        if (MetaFileAsset *meta = AssetManager::GetMetaFile(rawValue))
                            m_environmentSkyboxUUID = meta->uuid;
                    }
                }
            }
        }

        auto entities = _root["Entities"];

        LoadEntityNodes(_scriptRegistry, entities);
    }

    void Scene::LoadEntityNodes(std::vector<ScriptConf>& _scriptRegistry, YAML::Node &_entities, bool _copyUUID)
    {
        m_targetUUIDNewUUID.clear();
        m_entityConnectInfo.clear();
        std::vector<Canis::Entity*> newEntitys = {};

        if (_entities)
        {
            for (auto e : _entities)
            {
                newEntitys.push_back(&DecodeEntity(_scriptRegistry, e, _copyUUID));
            }

            for (auto eci : m_entityConnectInfo)
            {
                Canis::UUID uuid = eci.targetUUID;

                if (m_targetUUIDNewUUID.contains(uuid))
                    uuid = m_targetUUIDNewUUID[uuid];
                
                (*eci.variable) = GetEntityWithUUID(uuid);
            }

            // Keep bidirectional hierarchy links in sync after pointer remapping.
            for (Canis::Entity* entity : newEntitys)
            {
                if (entity == nullptr)
                    continue;

                if (entity->HasComponent<RectTransform>())
                {
                    RectTransform& transform = entity->GetComponent<RectTransform>();
                    if (transform.parent != nullptr)
                    {
                        if (transform.parent->HasComponent<RectTransform>())
                        {
                            RectTransform& parentTransform = transform.parent->GetComponent<RectTransform>();
                            auto& siblings = parentTransform.children;
                            if (std::find(siblings.begin(), siblings.end(), entity) == siblings.end())
                                siblings.push_back(entity);
                        }
                        else
                        {
                            transform.parent = nullptr;
                        }
                    }

                    for (Canis::Entity*& child : transform.children)
                    {
                        if (child == nullptr)
                            continue;

                        if (child->HasComponent<RectTransform>())
                        {
                            RectTransform& childTransform = child->GetComponent<RectTransform>();
                            childTransform.parent = entity;
                        }
                    }
                }

                if (entity->HasComponent<Transform3D>())
                {
                    Transform3D& transform = entity->GetComponent<Transform3D>();
                    if (transform.parent != nullptr)
                    {
                        if (transform.parent->HasComponent<Transform3D>())
                        {
                            Transform3D& parentTransform = transform.parent->GetComponent<Transform3D>();
                            auto& siblings = parentTransform.children;
                            if (std::find(siblings.begin(), siblings.end(), entity) == siblings.end())
                                siblings.push_back(entity);
                        }
                        else
                        {
                            transform.parent = nullptr;
                        }
                    }

                    for (Canis::Entity*& child : transform.children)
                    {
                        if (child == nullptr)
                            continue;

                        if (child->HasComponent<Transform3D>())
                        {
                            Transform3D& childTransform = child->GetComponent<Transform3D>();
                            childTransform.parent = entity;
                        }
                    }
                }
            }

            for (auto e : newEntitys)
            {
                for (ScriptableEntity* se : e->m_scriptComponents)
                {
                    if (se)
                    {
                        se->Create();
                    }
                }
            }
        }
    }

    Canis::Entity& Scene::DecodeEntity(std::vector<ScriptConf>& _scriptRegistry, YAML::Node _node, bool _copyUUID)
    {
        Entity& entity = *CreateEntity();
        
        if (_copyUUID) {
            entity.uuid = _node["Entity"].as<Canis::UUID>(0);
        } else {
            m_targetUUIDNewUUID[_node["Entity"].as<Canis::UUID>(0)] = entity.uuid;
        }
        
        entity.name = _node["Name"].as<std::string>("");
        entity.tag = _node["Tag"].as<std::string>("");

        for (int i = 0; i < _scriptRegistry.size(); i++)
        {
            if (_scriptRegistry[i].Decode)
            {
                _scriptRegistry[i].Decode(_node, entity, false);
            }
        }

        return entity;
    }

    void Scene::GetEntityAfterLoad(Canis::UUID _uuid, Canis::Entity* &_variable)
    {
        if (_uuid == 0)
        {
            _variable = nullptr;
            return;
        }

        if (app->GetEditor().GetMode() == EditorMode::PLAY)
        {
            _variable = GetEntityWithUUID(_uuid);
            return;
        }

        // editor edit mode
        EntityConnectInfo eci;
        eci.targetUUID = _uuid;
        eci.variable = &_variable;

        m_entityConnectInfo.push_back(eci);
    }

    void Scene::Save(std::vector<ScriptConf>& _scriptRegistry)
    {
        Debug::Log("Save Scene");
        
        YAML::Emitter out;

        out << EncodeScene(_scriptRegistry);

        if (m_path.size() > 0)
        {
            std::ofstream fout(m_path);
            fout << out.c_str();
        }
        else
        {
            std::ofstream fout(m_name);
            fout << out.c_str();
        }
    }

    YAML::Node Scene::EncodeScene(std::vector<ScriptConf>& _scriptRegistry)
    {
        YAML::Node node;

        YAML::Node environment;
        environment["ClearColor"] = m_window->GetClearColor();
        if ((uint64_t)m_environmentSkyboxUUID != 0)
        {
            YAML::Node skyboxAsset(YAML::NodeType::Map);
            skyboxAsset["uuid"] = (uint64_t)m_environmentSkyboxUUID;
            environment["SkyboxAsset"] = skyboxAsset;
        }
        node["Environment"] = environment;

        YAML::Node entities = YAML::Node(YAML::NodeType::Sequence);

        for(Entity* entity : m_entities)
        {
            if (!entity)
                continue;

            entities.push_back(EncodeEntity(_scriptRegistry, *entity));
        }

        node["Entities"] = entities;

        return node;
    }

    YAML::Node Scene::EncodeEntity(std::vector<ScriptConf>& _scriptRegistry, Entity &_entity)
    {
        YAML::Node node;
        node["Entity"] = _entity.uuid;
        node["Name"] = _entity.name;
        node["Tag"] = _entity.tag;

        for (int i = 0; i < _scriptRegistry.size(); i++)
            if (_scriptRegistry[i].Encode)
                _scriptRegistry[i].Encode(node, _entity);
        
        return node;
    }

    Entity* Scene::CreateEntity(std::string _name, std::string _tag)
    {
        Entity* entity = new Entity();
        entity->id = m_entities.size();
        entity->scene = this;
        entity->name = _name;
        entity->tag = _tag;
        entity->m_entityHandle = m_registry.create();

        // TODO : handle better
        for (int i = 0; i < m_entities.size(); i++)
        {
            if (m_entities[i] == nullptr)
            {
                if (i == 0)
                    Debug::Log("The Camera Just Died");
                m_entities[i] = entity;
                entity->id = i;
                QueueEntityForReady(entity->id);
                return entity;
            }
        }
        
        m_entities.push_back(entity);
        QueueEntityForReady(entity->id);
        return entity;
    }

    Entity* Scene::GetEntity(int _id)
    {
        if (_id > -1 && _id < m_entities.size())
            return m_entities[_id];
        
        // TODO : Handle Error
        return nullptr; 
    }

    Entity* Scene::GetEntityWithUUID(Canis::UUID _uuid)
    {
        for (int i = 0; i < m_entities.size(); i++)
            if (m_entities[i] != nullptr)
                if (m_entities[i]->uuid == _uuid)
                    return m_entities[i];

        
        // TODO : Handle Error
        return nullptr; 
    }

    Entity* Scene::FindEntityWithName(std::string _name)
    {
        for (Entity* entity : m_entities)
        {
            if (entity == nullptr)
                continue;
            
            if (entity->name == _name)
                return entity;
        }

        return nullptr;
    }

    Entity* Scene::GetEntityWithTag(std::string _tag)
    {
        for (Entity* entity : m_entities)
        {
            if (entity == nullptr)
                continue;
            
            if (entity->tag == _tag)
                return entity;
        }

        return nullptr;
    }

    std::vector<Entity*> Scene::GetEntitiesWithTag(std::string _tag)
    {
        std::vector<Entity*> entities = {};

        for (Entity* entity : m_entities)
        {
            if (entity == nullptr)
                continue;
            
            if (entity->tag == _tag)
                entities.push_back(entity);
        }

        return entities;
    }

    void Scene::Destroy(int _id)
    {
        if (_id < 0 || m_entities.size() <= _id)
        {
            Debug::Log("size %d, id %d", m_entities.size(), _id);
            return;
        }

        if (m_entities[_id] == nullptr)
        {
            Debug::Log("Why are you NULL");
            return;
        }

        if (m_isUpdating)
        {
            if (m_entities[_id]->active)
            {
                m_entities[_id]->active = false;
                m_entitiesToDestroy.push_back(_id);
            }
            return;
        }

        DestroyNow(_id);
    }

    void Scene::DestroyNow(int _id)
    {
        if (_id < 0 || m_entities.size() <= _id)
            return;

        Entity* entity = m_entities[_id];
        if (entity == nullptr)
            return;

        // Capture child ids before component teardown mutates hierarchy links.
        std::vector<int> childIdsToDestroy = {};
        auto queueChildForDestroy = [&](Entity* _child)
        {
            if (_child == nullptr || _child == entity)
                return;

            const int childId = _child->id;
            if (childId < 0 || childId >= static_cast<int>(m_entities.size()))
                return;

            if (m_entities[childId] != _child)
                return;

            if (std::find(childIdsToDestroy.begin(), childIdsToDestroy.end(), childId) == childIdsToDestroy.end())
                childIdsToDestroy.push_back(childId);
        };

        if (entity->HasComponent<RectTransform>())
        {
            RectTransform& rectTransform = entity->GetComponent<RectTransform>();
            if (rectTransform.parent != nullptr)
            {
                if (rectTransform.parent->HasComponent<RectTransform>())
                {
                    RectTransform& parentTransform = rectTransform.parent->GetComponent<RectTransform>();
                    auto& siblings = parentTransform.children;
                    siblings.erase(std::remove(siblings.begin(), siblings.end(), entity), siblings.end());
                }
                rectTransform.parent = nullptr;
            }

            for (Entity* child : rectTransform.children)
                queueChildForDestroy(child);
        }

        if (entity->HasComponent<Transform3D>())
        {
            Transform3D& transform3D = entity->GetComponent<Transform3D>();
            if (transform3D.parent != nullptr)
            {
                if (transform3D.parent->HasComponent<Transform3D>())
                {
                    Transform3D& parentTransform = transform3D.parent->GetComponent<Transform3D>();
                    auto& siblings = parentTransform.children;
                    siblings.erase(std::remove(siblings.begin(), siblings.end(), entity), siblings.end());
                }
                transform3D.parent = nullptr;
            }

            for (Entity* child : transform3D.children)
                queueChildForDestroy(child);
        }

        for (int childId : childIdsToDestroy)
            DestroyNow(childId);

        // Clear slot first to prevent recursive self-destroy during callbacks.
        m_entities[_id] = nullptr;

        entity->RemoveAllScripts();
        if (entity->m_entityHandle != entt::null && m_registry.valid(entity->m_entityHandle))
            m_registry.destroy(entity->m_entityHandle);
        delete entity;
    }

    void Scene::Destroy(Entity& _entity)
    {
        Destroy(_entity.id);
    }

    void Scene::QueueEntityForReady(int _id)
    {
        if (_id < 0 || _id >= (int)m_entities.size())
            return;

        m_entitiesToReady.push_back(_id);
    }

    void Scene::ReadySystem(System *_system)
    {
        _system->scene = this;
        _system->window = m_window;
        _system->inputManager = m_inputManager;
        //_system->time = m_time;
        //_system->camera = camera;
        m_systems.push_back(_system);
        m_systemTimings.push_back(SystemTiming{
            .name = _system->GetName(),
            .updateMs = 0.0f,
            .renderMs = 0.0f,
            .system = _system
        });
    }

    Scene::SystemTiming* Scene::GetSystemTiming(System* _system)
    {
        for (SystemTiming& timing : m_systemTimings)
        {
            if (timing.system == _system)
                return &timing;
        }

        return nullptr;
    }
}
