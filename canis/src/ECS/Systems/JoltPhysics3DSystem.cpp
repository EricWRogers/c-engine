#include <Canis/ECS/Systems/JoltPhysics3DSystem.hpp>

#include <Canis/Debug.hpp>
#include <Canis/Entity.hpp>
#include <Canis/Math.hpp>

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/AllowedDOFs.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <vector>

namespace Canis
{
    namespace
    {
        constexpr float kFixedTimeStep = 1.0f / 60.0f;
        constexpr int kMaxSubSteps = 4;
        constexpr int kCollisionSteps = 1;

        constexpr JPH::uint kMaxBodies = 65536;
        constexpr JPH::uint kNumBodyMutexes = 0;
        constexpr JPH::uint kMaxBodyPairs = 65536;
        constexpr JPH::uint kMaxContactConstraints = 10240;

        int g_joltInitRefCount = 0;

        namespace Layers
        {
            static constexpr JPH::ObjectLayer NON_MOVING = 0;
            static constexpr JPH::ObjectLayer MOVING = 1;
            static constexpr JPH::ObjectLayer NUM_LAYERS = 2;
        } // namespace Layers

        namespace BroadPhaseLayers
        {
            static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
            static constexpr JPH::BroadPhaseLayer MOVING(1);
            static constexpr JPH::uint NUM_LAYERS = 2;
        } // namespace BroadPhaseLayers

        class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter
        {
        public:
            bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override
            {
                switch (inObject1)
                {
                case Layers::NON_MOVING:
                    return inObject2 == Layers::MOVING;
                case Layers::MOVING:
                    return true;
                default:
                    return false;
                }
            }
        };

        class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
        {
        public:
            BPLayerInterfaceImpl()
            {
                m_objectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
                m_objectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
            }

            JPH::uint GetNumBroadPhaseLayers() const override
            {
                return BroadPhaseLayers::NUM_LAYERS;
            }

            JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
            {
                return m_objectToBroadPhase[inLayer];
            }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
            const char *GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
            {
                switch ((JPH::BroadPhaseLayer::Type)inLayer)
                {
                case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:
                    return "NON_MOVING";
                case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:
                    return "MOVING";
                default:
                    return "UNKNOWN";
                }
            }
#endif

        private:
            JPH::BroadPhaseLayer m_objectToBroadPhase[Layers::NUM_LAYERS];
        };

        class ObjectVsBroadPhaseLayerFilterImpl final : public JPH::ObjectVsBroadPhaseLayerFilter
        {
        public:
            bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override
            {
                switch (inLayer1)
                {
                case Layers::NON_MOVING:
                    return inLayer2 == BroadPhaseLayers::MOVING;
                case Layers::MOVING:
                    return true;
                default:
                    return false;
                }
            }
        };

        void InitJoltGlobal()
        {
            if (g_joltInitRefCount++ > 0)
                return;

            JPH::RegisterDefaultAllocator();
            JPH::Factory::sInstance = new JPH::Factory();
            JPH::RegisterTypes();
        }

        void ShutdownJoltGlobal()
        {
            if (g_joltInitRefCount <= 0)
                return;

            if (--g_joltInitRefCount > 0)
                return;

            JPH::UnregisterTypes();
            delete JPH::Factory::sInstance;
            JPH::Factory::sInstance = nullptr;
        }

        inline JPH::RVec3 ToJoltPosition(const Vector3 &_value)
        {
            return JPH::RVec3((double)_value.x, (double)_value.y, (double)_value.z);
        }

        inline Vector3 ToCanisPosition(const JPH::RVec3 &_value)
        {
            return Vector3((float)_value.GetX(), (float)_value.GetY(), (float)_value.GetZ());
        }

        inline JPH::Vec3 ToJoltVec3(const Vector3 &_value)
        {
            return JPH::Vec3(_value.x, _value.y, _value.z);
        }

        JPH::Quat ToJoltRotation(const Vector3 &_eulerRadians)
        {
            Matrix4 rotationMatrix = Matrix4(1.0f);
            rotationMatrix = glm::rotate(rotationMatrix, _eulerRadians.z, Vector3(0.0f, 0.0f, 1.0f));
            rotationMatrix = glm::rotate(rotationMatrix, _eulerRadians.y, Vector3(0.0f, 1.0f, 0.0f));
            rotationMatrix = glm::rotate(rotationMatrix, _eulerRadians.x, Vector3(1.0f, 0.0f, 0.0f));
            const glm::quat quat = glm::quat_cast(rotationMatrix);
            return JPH::Quat(quat.x, quat.y, quat.z, quat.w);
        }

        Vector3 ToCanisRotation(const JPH::Quat &_quat)
        {
            const glm::quat quat(_quat.GetW(), _quat.GetX(), _quat.GetY(), _quat.GetZ());
            return glm::eulerAngles(quat);
        }

        bool NearlyEqual(const Vector3 &_a, const Vector3 &_b, float _epsilon = 0.0005f)
        {
            return glm::all(glm::lessThanEqual(glm::abs(_a - _b), Vector3(_epsilon)));
        }

        bool NearlyZero(const Vector3 &_value, float _epsilon = 0.000001f)
        {
            return glm::all(glm::lessThanEqual(glm::abs(_value), Vector3(_epsilon)));
        }

        JPH::EMotionType ToMotionType(int _motionType)
        {
            switch (_motionType)
            {
            case Rigidbody3DMotionType::STATIC:
                return JPH::EMotionType::Static;
            case Rigidbody3DMotionType::KINEMATIC:
                return JPH::EMotionType::Kinematic;
            case Rigidbody3DMotionType::DYNAMIC:
            default:
                return JPH::EMotionType::Dynamic;
            }
        }

        JPH::ObjectLayer ToObjectLayer(JPH::EMotionType _motionType)
        {
            return (_motionType == JPH::EMotionType::Static) ? Layers::NON_MOVING : Layers::MOVING;
        }

        JPH::EAllowedDOFs BuildAllowedDOFs(const Rigidbody3D &_rigidbody)
        {
            JPH::EAllowedDOFs allowedDOFs =
                JPH::EAllowedDOFs::TranslationX |
                JPH::EAllowedDOFs::TranslationY |
                JPH::EAllowedDOFs::TranslationZ;

            if (!_rigidbody.lockRotationX)
                allowedDOFs = allowedDOFs | JPH::EAllowedDOFs::RotationX;

            if (!_rigidbody.lockRotationY)
                allowedDOFs = allowedDOFs | JPH::EAllowedDOFs::RotationY;

            if (!_rigidbody.lockRotationZ)
                allowedDOFs = allowedDOFs | JPH::EAllowedDOFs::RotationZ;

            return allowedDOFs;
        }

        void SetTransformFromWorldPose(Transform3D &_transform, const Vector3 &_worldPosition, const Vector3 &_worldRotation)
        {
            if (_transform.parent != nullptr)
            {
                if (Transform3D *parentTransform = ((_transform.parent) != nullptr && (_transform.parent)->HasComponent<Transform3D>() ? &(_transform.parent)->GetComponent<Transform3D>() : nullptr))
                {
                    const Matrix4 inverseParent = glm::inverse(parentTransform->GetModelMatrix());
                    const Vector4 localPosition4 = inverseParent * Vector4(_worldPosition, 1.0f);
                    _transform.position = Vector3(localPosition4.x, localPosition4.y, localPosition4.z);
                    _transform.rotation = _worldRotation - parentTransform->GetGlobalRotation();
                    return;
                }
            }

            _transform.position = _worldPosition;
            _transform.rotation = _worldRotation;
        }

        size_t BuildSettingsHash(const Transform3D &_transform, const Rigidbody3D &_rigidbody, const BoxCollider3D *_boxCollider, const SphereCollider3D *_sphereCollider, const CapsuleCollider3D *_capsuleCollider)
        {
            size_t hash = 0;
            hash = HashCombine(hash, std::hash<int>{}(_rigidbody.motionType));
            hash = HashCombine(hash, std::hash<float>{}(_rigidbody.mass));
            hash = HashCombine(hash, std::hash<float>{}(_rigidbody.friction));
            hash = HashCombine(hash, std::hash<float>{}(_rigidbody.restitution));
            hash = HashCombine(hash, std::hash<float>{}(_rigidbody.linearDamping));
            hash = HashCombine(hash, std::hash<float>{}(_rigidbody.angularDamping));
            hash = HashCombine(hash, std::hash<bool>{}(_rigidbody.useGravity));
            hash = HashCombine(hash, std::hash<bool>{}(_rigidbody.isSensor));
            hash = HashCombine(hash, std::hash<bool>{}(_rigidbody.allowSleeping));
            hash = HashCombine(hash, std::hash<bool>{}(_rigidbody.lockRotationX));
            hash = HashCombine(hash, std::hash<bool>{}(_rigidbody.lockRotationY));
            hash = HashCombine(hash, std::hash<bool>{}(_rigidbody.lockRotationZ));
            hash = HashCombine(hash, HashVector(_rigidbody.linearVelocity));
            hash = HashCombine(hash, HashVector(_rigidbody.angularVelocity));
            hash = HashCombine(hash, HashVector(glm::abs(_transform.GetGlobalScale())));

            if (_boxCollider != nullptr)
            {
                hash = HashCombine(hash, 101u);
                hash = HashCombine(hash, std::hash<bool>{}(_boxCollider->active));
                hash = HashCombine(hash, HashVector(_boxCollider->size));
            }
            else if (_sphereCollider != nullptr)
            {
                hash = HashCombine(hash, 102u);
                hash = HashCombine(hash, std::hash<bool>{}(_sphereCollider->active));
                hash = HashCombine(hash, std::hash<float>{}(_sphereCollider->radius));
            }
            else if (_capsuleCollider != nullptr)
            {
                hash = HashCombine(hash, 103u);
                hash = HashCombine(hash, std::hash<bool>{}(_capsuleCollider->active));
                hash = HashCombine(hash, std::hash<float>{}(_capsuleCollider->halfHeight));
                hash = HashCombine(hash, std::hash<float>{}(_capsuleCollider->radius));
            }
            else
            {
                hash = HashCombine(hash, 100u);
            }

            return hash;
        }

        JPH::RefConst<JPH::Shape> BuildShape(const Transform3D &_transform, const BoxCollider3D *_boxCollider, const SphereCollider3D *_sphereCollider, const CapsuleCollider3D *_capsuleCollider)
        {
            const Vector3 globalScale = glm::abs(_transform.GetGlobalScale());

            if (_boxCollider != nullptr)
            {
                const Vector3 halfExtents = glm::max((_boxCollider->size * globalScale) * 0.5f, Vector3(0.01f));
                JPH::BoxShapeSettings shapeSettings(ToJoltVec3(halfExtents));
                JPH::Shape::ShapeResult shapeResult = shapeSettings.Create();
                if (shapeResult.HasError())
                {
                    Debug::Log("Jolt BoxCollider3D shape error: %s", shapeResult.GetError().c_str());
                    return nullptr;
                }
                return shapeResult.Get();
            }

            if (_sphereCollider != nullptr)
            {
                const float scale = glm::max(globalScale.x, glm::max(globalScale.y, globalScale.z));
                const float radius = glm::max(0.01f, _sphereCollider->radius * scale);
                JPH::SphereShapeSettings shapeSettings(radius);
                JPH::Shape::ShapeResult shapeResult = shapeSettings.Create();
                if (shapeResult.HasError())
                {
                    Debug::Log("Jolt SphereCollider3D shape error: %s", shapeResult.GetError().c_str());
                    return nullptr;
                }
                return shapeResult.Get();
            }

            if (_capsuleCollider != nullptr)
            {
                const float radiusScale = glm::max(globalScale.x, globalScale.z);
                const float radius = glm::max(0.01f, _capsuleCollider->radius * radiusScale);
                const float halfHeight = glm::max(0.01f, _capsuleCollider->halfHeight * globalScale.y);
                JPH::CapsuleShapeSettings shapeSettings(halfHeight, radius);
                JPH::Shape::ShapeResult shapeResult = shapeSettings.Create();
                if (shapeResult.HasError())
                {
                    Debug::Log("Jolt CapsuleCollider3D shape error: %s", shapeResult.GetError().c_str());
                    return nullptr;
                }
                return shapeResult.Get();
            }

            return nullptr;
        }
    } // namespace

    struct JoltPhysics3DSystem::Impl
    {
        struct BodyRuntimeData
        {
            JPH::BodyID bodyID;
            size_t settingsHash = 0;
            Vector3 syncedLocalPosition = Vector3(0.0f);
            Vector3 syncedLocalRotation = Vector3(0.0f);
            bool hasSyncedTransform = false;
        };

        std::unique_ptr<BPLayerInterfaceImpl> broadPhaseLayerInterface = nullptr;
        std::unique_ptr<ObjectVsBroadPhaseLayerFilterImpl> objectVsBroadPhaseLayerFilter = nullptr;
        std::unique_ptr<ObjectLayerPairFilterImpl> objectLayerPairFilter = nullptr;
        std::unique_ptr<JPH::PhysicsSystem> physicsSystem = nullptr;
        std::unique_ptr<JPH::TempAllocatorImpl> tempAllocator = nullptr;
        std::unique_ptr<JPH::JobSystemThreadPool> jobSystem = nullptr;
        JPH::BodyInterface *bodyInterface = nullptr;
        std::unordered_map<entt::entity, BodyRuntimeData> bodies = {};
        float accumulator = 0.0f;
        bool initialized = false;

        void Create()
        {
            if (initialized)
                return;

            InitJoltGlobal();

            broadPhaseLayerInterface = std::make_unique<BPLayerInterfaceImpl>();
            objectVsBroadPhaseLayerFilter = std::make_unique<ObjectVsBroadPhaseLayerFilterImpl>();
            objectLayerPairFilter = std::make_unique<ObjectLayerPairFilterImpl>();

            physicsSystem = std::make_unique<JPH::PhysicsSystem>();
            physicsSystem->Init(
                kMaxBodies,
                kNumBodyMutexes,
                kMaxBodyPairs,
                kMaxContactConstraints,
                *broadPhaseLayerInterface,
                *objectVsBroadPhaseLayerFilter,
                *objectLayerPairFilter);

            tempAllocator = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);

            const JPH::uint workerCount = (std::thread::hardware_concurrency() > 1)
                ? (std::thread::hardware_concurrency() - 1)
                : 1;
            jobSystem = std::make_unique<JPH::JobSystemThreadPool>(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, workerCount);
            bodyInterface = &physicsSystem->GetBodyInterface();
            accumulator = 0.0f;
            initialized = true;
        }

        void Destroy()
        {
            if (!initialized)
                return;

            for (auto &entry : bodies)
                RemoveBodyFromWorld(entry.second.bodyID);
            bodies.clear();

            bodyInterface = nullptr;
            jobSystem.reset();
            tempAllocator.reset();
            physicsSystem.reset();
            objectLayerPairFilter.reset();
            objectVsBroadPhaseLayerFilter.reset();
            broadPhaseLayerInterface.reset();
            accumulator = 0.0f;

            ShutdownJoltGlobal();
            initialized = false;
        }

        void RemoveBodyFromWorld(const JPH::BodyID &_bodyID)
        {
            if (bodyInterface == nullptr || _bodyID.IsInvalid())
                return;

            if (bodyInterface->IsAdded(_bodyID))
                bodyInterface->RemoveBody(_bodyID);
            bodyInterface->DestroyBody(_bodyID);
        }

        void RemoveBody(entt::entity _entityHandle)
        {
            auto bodyIt = bodies.find(_entityHandle);
            if (bodyIt == bodies.end())
                return;

            RemoveBodyFromWorld(bodyIt->second.bodyID);
            bodies.erase(bodyIt);
        }

        void ClearPendingForces(Rigidbody3D &_rigidbody)
        {
            _rigidbody.pendingForce = Vector3(0.0f);
            _rigidbody.pendingAcceleration = Vector3(0.0f);
            _rigidbody.pendingImpulse = Vector3(0.0f);
            _rigidbody.pendingVelocityChange = Vector3(0.0f);
        }

        void ApplyPendingForces(const JPH::BodyID &_bodyID, Rigidbody3D &_rigidbody)
        {
            if (bodyInterface == nullptr || _bodyID.IsInvalid() || !bodyInterface->IsAdded(_bodyID))
                return;

            bool applied = false;
            const float mass = glm::max(0.001f, _rigidbody.mass);

            if (!NearlyZero(_rigidbody.pendingForce))
            {
                bodyInterface->AddForce(_bodyID, ToJoltVec3(_rigidbody.pendingForce));
                _rigidbody.pendingForce = Vector3(0.0f);
                applied = true;
            }

            if (!NearlyZero(_rigidbody.pendingAcceleration))
            {
                bodyInterface->AddForce(_bodyID, ToJoltVec3(_rigidbody.pendingAcceleration * mass));
                _rigidbody.pendingAcceleration = Vector3(0.0f);
                applied = true;
            }

            if (!NearlyZero(_rigidbody.pendingImpulse))
            {
                bodyInterface->AddImpulse(_bodyID, ToJoltVec3(_rigidbody.pendingImpulse));
                _rigidbody.pendingImpulse = Vector3(0.0f);
                applied = true;
            }

            if (!NearlyZero(_rigidbody.pendingVelocityChange))
            {
                bodyInterface->AddImpulse(_bodyID, ToJoltVec3(_rigidbody.pendingVelocityChange * mass));
                _rigidbody.pendingVelocityChange = Vector3(0.0f);
                applied = true;
            }

            if (applied)
                bodyInterface->ActivateBody(_bodyID);
        }

        bool EnsureBodyForEntity(entt::registry &_registry, entt::entity _entityHandle)
        {
            Transform3D *transform = _registry.try_get<Transform3D>(_entityHandle);
            Rigidbody3D *rigidbody = _registry.try_get<Rigidbody3D>(_entityHandle);
            if (transform == nullptr || rigidbody == nullptr)
            {
                RemoveBody(_entityHandle);
                return false;
            }

            Entity *entity = rigidbody->entity != nullptr ? rigidbody->entity : transform->entity;
            if (entity == nullptr || !entity->active || !rigidbody->active)
            {
                RemoveBody(_entityHandle);
                return false;
            }

            BoxCollider3D *boxCollider = _registry.try_get<BoxCollider3D>(_entityHandle);
            SphereCollider3D *sphereCollider = _registry.try_get<SphereCollider3D>(_entityHandle);
            CapsuleCollider3D *capsuleCollider = _registry.try_get<CapsuleCollider3D>(_entityHandle);

            if (boxCollider != nullptr && !boxCollider->active)
                boxCollider = nullptr;
            if (sphereCollider != nullptr && !sphereCollider->active)
                sphereCollider = nullptr;
            if (capsuleCollider != nullptr && !capsuleCollider->active)
                capsuleCollider = nullptr;

            if (boxCollider == nullptr && sphereCollider == nullptr && capsuleCollider == nullptr)
            {
                RemoveBody(_entityHandle);
                return false;
            }

            const size_t settingsHash = BuildSettingsHash(*transform, *rigidbody, boxCollider, sphereCollider, capsuleCollider);
            auto bodyIt = bodies.find(_entityHandle);
            const bool needsRecreate = (bodyIt == bodies.end()) || (bodyIt->second.settingsHash != settingsHash);

            if (!needsRecreate)
                return true;

            if (bodyIt != bodies.end())
                RemoveBodyFromWorld(bodyIt->second.bodyID);

            JPH::RefConst<JPH::Shape> shape = BuildShape(*transform, boxCollider, sphereCollider, capsuleCollider);
            if (shape == nullptr)
            {
                RemoveBody(_entityHandle);
                return false;
            }

            const JPH::EMotionType motionType = ToMotionType(rigidbody->motionType);
            JPH::BodyCreationSettings bodySettings(
                shape.GetPtr(),
                ToJoltPosition(transform->GetGlobalPosition()),
                ToJoltRotation(transform->GetGlobalRotation()),
                motionType,
                ToObjectLayer(motionType));

            bodySettings.mAllowSleeping = rigidbody->allowSleeping;
            bodySettings.mIsSensor = rigidbody->isSensor;
            bodySettings.mFriction = glm::max(0.0f, rigidbody->friction);
            bodySettings.mRestitution = glm::max(0.0f, rigidbody->restitution);
            bodySettings.mLinearDamping = glm::max(0.0f, rigidbody->linearDamping);
            bodySettings.mAngularDamping = glm::max(0.0f, rigidbody->angularDamping);
            bodySettings.mGravityFactor = rigidbody->useGravity ? 1.0f : 0.0f;
            bodySettings.mAllowedDOFs = BuildAllowedDOFs(*rigidbody);
            bodySettings.mLinearVelocity = ToJoltVec3(rigidbody->linearVelocity);
            bodySettings.mAngularVelocity = ToJoltVec3(rigidbody->angularVelocity);

            if (motionType == JPH::EMotionType::Dynamic)
            {
                bodySettings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
                bodySettings.mMassPropertiesOverride.mMass = glm::max(0.001f, rigidbody->mass);
            }

            const JPH::EActivation activation = (motionType == JPH::EMotionType::Static)
                ? JPH::EActivation::DontActivate
                : JPH::EActivation::Activate;

            const JPH::BodyID bodyID = bodyInterface->CreateAndAddBody(bodySettings, activation);
            if (bodyID.IsInvalid())
            {
                Debug::Log("JoltPhysics3DSystem: failed to create body for entity '%s'.", entity->name.c_str());
                RemoveBody(_entityHandle);
                return false;
            }

            BodyRuntimeData runtimeData;
            runtimeData.bodyID = bodyID;
            runtimeData.settingsHash = settingsHash;
            runtimeData.syncedLocalPosition = transform->position;
            runtimeData.syncedLocalRotation = transform->rotation;
            runtimeData.hasSyncedTransform = true;
            bodies[_entityHandle] = runtimeData;
            return true;
        }

        std::vector<entt::entity> SyncBodiesBeforeStep(entt::registry &_registry)
        {
            std::vector<entt::entity> activeBodies = {};
            std::unordered_set<entt::entity> expected = {};

            auto rigidbodyView = _registry.view<Rigidbody3D, Transform3D>();
            for (const entt::entity entityHandle : rigidbodyView)
            {
                if (!EnsureBodyForEntity(_registry, entityHandle))
                    continue;

                expected.insert(entityHandle);
                activeBodies.push_back(entityHandle);
            }

            std::vector<entt::entity> staleEntities = {};
            staleEntities.reserve(bodies.size());
            for (const auto &entry : bodies)
            {
                const entt::entity entityHandle = entry.first;
                if (!_registry.valid(entityHandle) || expected.find(entityHandle) == expected.end())
                    staleEntities.push_back(entityHandle);
            }

            for (const entt::entity entityHandle : staleEntities)
                RemoveBody(entityHandle);

            for (const entt::entity entityHandle : activeBodies)
            {
                auto bodyIt = bodies.find(entityHandle);
                Transform3D *transform = _registry.try_get<Transform3D>(entityHandle);
                Rigidbody3D *rigidbody = _registry.try_get<Rigidbody3D>(entityHandle);
                if (bodyIt == bodies.end() || transform == nullptr || rigidbody == nullptr)
                    continue;

                BodyRuntimeData &runtimeData = bodyIt->second;
                const JPH::EMotionType motionType = ToMotionType(rigidbody->motionType);

                const bool transformEdited = !runtimeData.hasSyncedTransform
                    || !NearlyEqual(transform->position, runtimeData.syncedLocalPosition)
                    || !NearlyEqual(transform->rotation, runtimeData.syncedLocalRotation);

                if (motionType == JPH::EMotionType::Static || motionType == JPH::EMotionType::Kinematic || transformEdited)
                {
                    const JPH::EActivation activation = (motionType == JPH::EMotionType::Static)
                        ? JPH::EActivation::DontActivate
                        : JPH::EActivation::Activate;
                    bodyInterface->SetPositionAndRotation(
                        runtimeData.bodyID,
                        ToJoltPosition(transform->GetGlobalPosition()),
                        ToJoltRotation(transform->GetGlobalRotation()),
                        activation);

                    runtimeData.syncedLocalPosition = transform->position;
                    runtimeData.syncedLocalRotation = transform->rotation;
                    runtimeData.hasSyncedTransform = true;
                }

                if (motionType == JPH::EMotionType::Dynamic)
                    ApplyPendingForces(runtimeData.bodyID, *rigidbody);
                else
                    ClearPendingForces(*rigidbody);
            }

            return activeBodies;
        }

        void SyncBodiesAfterStep(entt::registry &_registry, const std::vector<entt::entity> &_activeBodies)
        {
            for (const entt::entity entityHandle : _activeBodies)
            {
                auto bodyIt = bodies.find(entityHandle);
                Transform3D *transform = _registry.try_get<Transform3D>(entityHandle);
                Rigidbody3D *rigidbody = _registry.try_get<Rigidbody3D>(entityHandle);
                if (bodyIt == bodies.end() || transform == nullptr || rigidbody == nullptr)
                    continue;

                const JPH::EMotionType motionType = ToMotionType(rigidbody->motionType);
                if (motionType != JPH::EMotionType::Dynamic)
                    continue;

                BodyRuntimeData &runtimeData = bodyIt->second;
                if (runtimeData.bodyID.IsInvalid() || !bodyInterface->IsAdded(runtimeData.bodyID))
                    continue;

                const Vector3 worldPosition = ToCanisPosition(bodyInterface->GetCenterOfMassPosition(runtimeData.bodyID));
                const Vector3 worldRotation = ToCanisRotation(bodyInterface->GetRotation(runtimeData.bodyID));

                SetTransformFromWorldPose(*transform, worldPosition, worldRotation);
                runtimeData.syncedLocalPosition = transform->position;
                runtimeData.syncedLocalRotation = transform->rotation;
                runtimeData.hasSyncedTransform = true;
            }
        }

        void Update(entt::registry &_registry, float _deltaTime)
        {
            if (physicsSystem == nullptr || bodyInterface == nullptr)
                return;

            std::vector<entt::entity> activeBodies = SyncBodiesBeforeStep(_registry);

            accumulator += glm::max(0.0f, _deltaTime);
            accumulator = glm::min(accumulator, kFixedTimeStep * (float)kMaxSubSteps);

            while (accumulator >= kFixedTimeStep)
            {
                physicsSystem->Update(kFixedTimeStep, kCollisionSteps, tempAllocator.get(), jobSystem.get());
                accumulator -= kFixedTimeStep;
            }

            SyncBodiesAfterStep(_registry, activeBodies);
        }
    };

    JoltPhysics3DSystem::JoltPhysics3DSystem() : System()
    {
        m_name = type_name<JoltPhysics3DSystem>();
        m_impl = new Impl();
    }

    JoltPhysics3DSystem::~JoltPhysics3DSystem()
    {
        OnDestroy();
    }

    void JoltPhysics3DSystem::Create()
    {
        if (m_impl == nullptr)
            m_impl = new Impl();

        m_impl->Create();
    }

    void JoltPhysics3DSystem::Ready()
    {
    }

    void JoltPhysics3DSystem::Update(entt::registry &_registry, float _deltaTime)
    {
        if (m_impl == nullptr)
            return;

        m_impl->Update(_registry, _deltaTime);
    }

    void JoltPhysics3DSystem::OnDestroy()
    {
        if (m_impl == nullptr)
            return;

        m_impl->Destroy();
        delete m_impl;
        m_impl = nullptr;
    }
} // namespace Canis
