#pragma once

#include <cstdint>
#include <functional>
#include <utility>

#include "EcsTypes.hpp"
#include "EntityManager.hpp"

namespace engine::ecs
{
	class CEntity
	{
		public:

			CEntity(entity_id_t _entity_id, CEntityManager& _entity_manager);
			void Destroy();

			template<typename T, typename... Args>
			[[nodiscard]] T& AddComponent(Args&&... _args)
			{
				return m_EntityManager.AddComponent<T>(m_EntityId, std::forward<Args>(_args)...);
			}

			template<typename T>
			void RemoveComponent()
			{
				m_EntityManager.RemoveComponent<T>(m_EntityId);
			}

			template<typename T>
			[[nodiscard]] T& GetComponent()
			{
				return m_EntityManager.GetComponent<T>(m_EntityId);
			}

			template<typename T>
			[[nodiscard]] const T& GetComponent() const
			{
				return m_EntityManager.GetComponent<T>(m_EntityId);
			}

			template<typename T>
			[[nodiscard]] T* FindComponent()
			{
				return m_EntityManager.FindComponent<T>(m_EntityId);
			}

			template<typename T>
			[[nodiscard]] const T* FindComponent() const
			{
				return m_EntityManager.FindComponent<T>(m_EntityId);
			}

			template<typename T>
			[[nodiscard]] bool HasComponent() const
			{
				return m_EntityManager.HasComponent<T>(m_EntityId);
			}

			[[nodiscard]] entity_id_t GetId() const { return m_EntityId; }
			[[nodiscard]] bool IsValid() const;

			[[nodiscard]] bool operator==(const CEntity& other) const { return m_EntityId == other.m_EntityId; }
			[[nodiscard]] bool operator!=(const CEntity& other) const { return m_EntityId != other.m_EntityId; }

		private:

			CEntityManager& m_EntityManager;
			entity_id_t m_EntityId;
	};
}

template<>
struct std::hash<engine::ecs::CEntity>
{
	std::size_t operator()(const engine::ecs::CEntity& _e) const noexcept { return _e.GetId(); }
};