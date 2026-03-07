#include <Canis/ECS/RuntimeECS.hpp>

namespace Canis
{
    bool RuntimeECS::IsEntityValid(u32 _entityId) const
    {
        return _entityId < m_masks.size();
    }

    void RuntimeECS::EnsureEntityCapacity(u32 _entityId)
    {
        if (_entityId < m_masks.size())
            return;

        const u32 newSize = _entityId + 1;
        m_masks.resize(newSize, 0);

        for (ComponentStorage& storage : m_components)
        {
            if (!storage.registered)
                continue;

            storage.sparse.resize(newSize, INVALID_INDEX);
        }
    }

    void RuntimeECS::RegisterComponent(u32 _componentIndex, u64 _componentMask)
    {
        if (_componentMask == 0)
            return;

        if (_componentIndex >= m_components.size())
            m_components.resize(_componentIndex + 1);

        ComponentStorage& storage = m_components[_componentIndex];
        if (storage.registered && storage.maskBit == _componentMask)
            return;

        storage.registered = true;
        storage.maskBit = _componentMask;
        storage.sparse.resize(m_masks.size(), INVALID_INDEX);
        ++m_version;
    }

    void RuntimeECS::UnregisterComponent(u32 _componentIndex)
    {
        if (_componentIndex >= m_components.size())
            return;

        ComponentStorage& storage = m_components[_componentIndex];
        if (!storage.registered)
            return;

        for (u32 entityId : storage.denseEntityIds)
        {
            if (entityId < m_masks.size())
                m_masks[entityId] &= ~storage.maskBit;
        }

        storage.sparse.clear();
        storage.denseEntityIds.clear();
        storage.denseComponents.clear();
        storage.maskBit = 0;
        storage.registered = false;
        ++m_version;
    }

    ScriptableEntity* RuntimeECS::AddComponent(u32 _componentIndex, u64 _componentMask, u32 _entityId, ScriptableEntity* _component)
    {
        if (_component == nullptr || _componentMask == 0)
            return nullptr;

        EnsureEntityCapacity(_entityId);
        RegisterComponent(_componentIndex, _componentMask);

        ComponentStorage& storage = m_components[_componentIndex];
        if (_entityId >= storage.sparse.size())
            storage.sparse.resize(_entityId + 1, INVALID_INDEX);

        u32 denseIndex = storage.sparse[_entityId];
        if (denseIndex != INVALID_INDEX)
            return storage.denseComponents[denseIndex];

        storage.denseEntityIds.push_back(_entityId);
        storage.denseComponents.push_back(_component);

        denseIndex = static_cast<u32>(storage.denseComponents.size() - 1);
        storage.sparse[_entityId] = denseIndex;
        m_masks[_entityId] |= _componentMask;
        ++m_version;
        return _component;
    }

    ScriptableEntity* RuntimeECS::GetComponent(u32 _componentIndex, u32 _entityId) const
    {
        if (_componentIndex >= m_components.size() || !IsEntityValid(_entityId))
            return nullptr;

        const ComponentStorage& storage = m_components[_componentIndex];
        if (!storage.registered || _entityId >= storage.sparse.size())
            return nullptr;

        const u32 denseIndex = storage.sparse[_entityId];
        if (denseIndex == INVALID_INDEX || denseIndex >= storage.denseComponents.size())
            return nullptr;

        return storage.denseComponents[denseIndex];
    }

    ScriptableEntity* RuntimeECS::RemoveComponent(u32 _componentIndex, u32 _entityId)
    {
        if (_componentIndex >= m_components.size() || !IsEntityValid(_entityId))
            return nullptr;

        ComponentStorage& storage = m_components[_componentIndex];
        if (!storage.registered || _entityId >= storage.sparse.size())
            return nullptr;

        const u32 denseIndex = storage.sparse[_entityId];
        if (denseIndex == INVALID_INDEX || denseIndex >= storage.denseComponents.size())
            return nullptr;

        ScriptableEntity* removed = storage.denseComponents[denseIndex];
        const u32 lastIndex = static_cast<u32>(storage.denseComponents.size() - 1);

        if (denseIndex != lastIndex)
        {
            storage.denseComponents[denseIndex] = storage.denseComponents[lastIndex];
            const u32 swappedEntityId = storage.denseEntityIds[lastIndex];
            storage.denseEntityIds[denseIndex] = swappedEntityId;
            storage.sparse[swappedEntityId] = denseIndex;
        }

        storage.denseComponents.pop_back();
        storage.denseEntityIds.pop_back();
        storage.sparse[_entityId] = INVALID_INDEX;
        m_masks[_entityId] &= ~storage.maskBit;
        ++m_version;
        return removed;
    }

    void RuntimeECS::InitView(RuntimeECSView& _view, u64 _requiredMask, u32 _capacity) const
    {
        _view.requiredMask = _requiredMask;
        _view.lastVersion = 0;
        _view.entities.clear();
        _view.entities.reserve(_capacity);
        UpdateView(_view);
    }

    void RuntimeECS::UpdateView(RuntimeECSView& _view) const
    {
        if (_view.lastVersion == m_version)
            return;

        _view.entities.clear();

        if (_view.requiredMask != 0)
        {
            const u32 entityCount = static_cast<u32>(m_masks.size());
            for (u32 entityId = 0; entityId < entityCount; ++entityId)
            {
                if ((m_masks[entityId] & _view.requiredMask) == _view.requiredMask)
                    _view.entities.push_back(entityId);
            }
        }

        _view.lastVersion = m_version;
    }

    u64 RuntimeECS::GetEntityMask(u32 _entityId) const
    {
        if (!IsEntityValid(_entityId))
            return 0;

        return m_masks[_entityId];
    }

    u32 RuntimeECS::GetEntityCount() const
    {
        return static_cast<u32>(m_masks.size());
    }
}
