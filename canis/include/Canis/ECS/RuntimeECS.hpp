#pragma once

#include <vector>
#include <limits>

#include <Canis/Data/Types.hpp>

namespace Canis
{
    class ScriptableEntity;

    struct RuntimeECSView
    {
        u64 requiredMask = 0;
        u64 lastVersion = 0;
        std::vector<u32> entities = {};
    };

    class RuntimeECS
    {
    public:
        void EnsureEntityCapacity(u32 _entityId);

        void RegisterComponent(u32 _componentIndex, u64 _componentMask);
        void UnregisterComponent(u32 _componentIndex);

        ScriptableEntity* AddComponent(u32 _componentIndex, u64 _componentMask, u32 _entityId, ScriptableEntity* _component);
        ScriptableEntity* GetComponent(u32 _componentIndex, u32 _entityId) const;
        ScriptableEntity* RemoveComponent(u32 _componentIndex, u32 _entityId);

        void InitView(RuntimeECSView& _view, u64 _requiredMask, u32 _capacity = 64) const;
        void UpdateView(RuntimeECSView& _view) const;

        u64 GetEntityMask(u32 _entityId) const;
        u32 GetEntityCount() const;
        u64 GetVersion() const { return m_version; }

    private:
        struct ComponentStorage
        {
            bool registered = false;
            u64 maskBit = 0;
            std::vector<u32> sparse = {};
            std::vector<u32> denseEntityIds = {};
            std::vector<ScriptableEntity*> denseComponents = {};
        };

        static constexpr u32 INVALID_INDEX = std::numeric_limits<u32>::max();

        std::vector<u64> m_masks = {};
        std::vector<ComponentStorage> m_components = {};
        u64 m_version = 1;

        bool IsEntityValid(u32 _entityId) const;
    };
}
