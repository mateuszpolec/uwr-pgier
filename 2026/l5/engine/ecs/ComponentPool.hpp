#pragma once

#include <vector>
#include <cstdint>
#include <utility>

#include "EcsTypes.hpp"
#include "engine/core/Assert.hpp"

namespace engine::ecs
{
	///////////////////////////////////////////////////////////////////////////////////////
	class IComponentPool
	{
		public:

			virtual ~IComponentPool() = default;
			virtual void RemoveComponent(entity_id_t _entity_id) = 0;
			[[nodiscard]] virtual bool HasComponent(entity_id_t _entity_id) const = 0;
			[[nodiscard]] virtual size_t Size() const = 0;
			[[nodiscard]] virtual const std::vector<entity_id_t>& GetEntities() const = 0;
	};

	///////////////////////////////////////////////////////////////////////////////////////
	template<typename T>
	class CComponentPool final : public IComponentPool
	{
		public:

			template<typename... Args>
			T& AddComponent(entity_id_t _entity_id, Args&&... _args)
			{
				if (HasComponent(_entity_id))
				{
					return m_Components[m_Sparse[_entity_id]] = T{ std::forward<Args>(_args)... };
				}

				if (m_Sparse.size() <= _entity_id)
				{
					m_Sparse.resize(_entity_id + 1, INVALID_INDEX);
				}

				m_Sparse[_entity_id] = static_cast<std::uint32_t>(m_Dense.size());
				m_Dense.push_back(_entity_id);
				return m_Components.emplace_back(std::forward<Args>(_args)...);
			}

			void RemoveComponent(entity_id_t _entity_id) override
			{
				if (!HasComponent(_entity_id))
				{
					return;
				}

				const std::uint32_t index = m_Sparse[_entity_id];
				const std::uint32_t last_index = static_cast<std::uint32_t>(m_Dense.size() - 1);

				m_Dense[index] = m_Dense[last_index];
				m_Components[index] = std::move(m_Components[last_index]);
				m_Sparse[m_Dense[index]] = index;

				m_Dense.pop_back();
				m_Components.pop_back();
				m_Sparse[_entity_id] = INVALID_INDEX;
			}

			[[nodiscard]] bool HasComponent(entity_id_t _entity_id) const override
			{
				return _entity_id < m_Sparse.size() && m_Sparse[_entity_id] != INVALID_INDEX && m_Dense[m_Sparse[_entity_id]] == _entity_id;
			}
			[[nodiscard]] size_t Size() const override
			{
				return m_Dense.size();
			}

			[[nodiscard]] const std::vector<entity_id_t>& GetEntities() const override
			{
				return m_Dense;
			}

			[[nodiscard]] T& GetComponent(entity_id_t _entity_id)
			{
				ASSERT(HasComponent(_entity_id), "Entity does not have this component");
				return m_Components[m_Sparse[_entity_id]];
			}

			[[nodiscard]] const T& GetComponent(entity_id_t _entity_id) const
			{
				ASSERT(HasComponent(_entity_id), "Entity does not have this component");
				return m_Components[m_Sparse[_entity_id]];
			}

			[[nodiscard]] T* FindComponent(entity_id_t _entity_id)
			{
				if (!HasComponent(_entity_id))
				{
					return nullptr;
				}

				return &m_Components[m_Sparse[_entity_id]];
			}

			[[nodiscard]] const T* FindComponent(entity_id_t _entity_id) const
			{
				if (!HasComponent(_entity_id))
				{
					return nullptr;
				}

				return &m_Components[m_Sparse[_entity_id]];
			}

		private:

			static constexpr std::uint32_t INVALID_INDEX = static_cast<std::uint32_t>(-1);

			std::vector<std::uint32_t> m_Sparse;
			std::vector<entity_id_t> m_Dense;
			std::vector<T> m_Components;
	};
}