#pragma once

#include <unordered_set>
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <utility>

#include "ComponentPool.hpp"
#include "EcsTypes.hpp"

namespace engine::ecs
{
	class CEntity;

	class CEntityManager
	{
	public:

		[[nodiscard]] CEntity CreateEntity();
		void DestroyEntity(entity_id_t _entity_id);
		[[nodiscard]] bool IsValid(entity_id_t _entity_id) const;

		template<typename T, typename... Args>
		[[nodiscard]] T& AddComponent(entity_id_t _entity_id, Args&&... _args)
		{
			return GetComponentPool<T>().AddComponent(_entity_id, std::forward<Args>(_args)...);
		}

		template<typename T>
		void RemoveComponent(entity_id_t _entity_id)
		{
			if (auto* pool = FindComponentPool<T>())
			{
				pool->RemoveComponent(_entity_id);
			}
		}

		template<typename T>
		[[nodiscard]] T& GetComponent(entity_id_t _entity_id)
		{
			return GetComponentPool<T>().GetComponent(_entity_id);
		}

		template<typename T>
		[[nodiscard]] const T& GetComponent(entity_id_t _entity_id) const
		{
			const auto* pool = FindComponentPool<T>();
			ASSERT(pool != nullptr, "Component pool does not exist");
			return pool->GetComponent(_entity_id);
		}

		template<typename T>
		[[nodiscard]] T* FindComponent(entity_id_t _entity_id)
		{
			auto* pool = FindComponentPool<T>();
			return pool ? pool->FindComponent(_entity_id) : nullptr;
		}

		template<typename T>
		[[nodiscard]] const T* FindComponent(entity_id_t _entity_id) const
		{
			const auto* pool = FindComponentPool<T>();
			return pool ? pool->FindComponent(_entity_id) : nullptr;
		}

		template<typename T>
		[[nodiscard]] bool HasComponent(entity_id_t _entity_id) const
		{
			const auto* pool = FindComponentPool<T>();
			return pool && pool->HasComponent(_entity_id);
		}

		template<typename T>
		[[nodiscard]] size_t GetComponentPoolSize() const
		{
			const auto* pool = FindComponentPool<T>();
			return pool ? pool->Size() : 0;
		}

		template<typename First, typename... Rest, typename Fn>
		void ForEach(Fn&& _fn)
		{
			auto& first_pool = GetComponentPool<First>();
			const auto& entities = first_pool.GetEntities();
			for (const auto entity_id : entities)
			{
				if ((GetComponentPool<Rest>().HasComponent(entity_id) && ...))
				{
					_fn(entity_id, first_pool.GetComponent(entity_id), GetComponentPool<Rest>().GetComponent(entity_id)...);
				}
			}
		}

	private:

		template<typename T>
		[[nodiscard]] CComponentPool<T>& GetComponentPool()
		{
			const std::type_index type_index = typeid(T);

			auto it = m_ComponentPools.find(type_index);
			if (it == m_ComponentPools.end())
			{
				it = m_ComponentPools.emplace(type_index, std::make_unique<CComponentPool<T>>()).first;
			}

			return static_cast<CComponentPool<T>&>(*it->second);
		}

		template<typename T>
		[[nodiscard]] CComponentPool<T>* FindComponentPool()
		{
			const auto it = m_ComponentPools.find(typeid(T));
			return it != m_ComponentPools.end() ? static_cast<CComponentPool<T>*>(it->second.get()) : nullptr;
		}

		template<typename T>
		[[nodiscard]] const CComponentPool<T>* FindComponentPool() const
		{
			const auto it = m_ComponentPools.find(typeid(T));
			return it != m_ComponentPools.end() ? static_cast<const CComponentPool<T>*>(it->second.get()) : nullptr;
		}

	private:

		entity_id_t m_NextEntityId = 0;
		std::unordered_set<entity_id_t> m_Entities;
		std::unordered_map<std::type_index, std::unique_ptr<IComponentPool>> m_ComponentPools;
	};
}