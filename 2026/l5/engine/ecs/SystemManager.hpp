#pragma once

#include <memory>
#include <vector>

#include <SFML/Graphics/RenderTarget.hpp>

#include "EntityManager.hpp"
#include "engine/core/NonCopyable.hpp"

namespace engine::ecs
{
///////////////////////////////////////////////////////////////////////////////////////
	class ISystem
	{
	public:

		virtual ~ISystem() = default;
		virtual void Frame(CEntityManager& _components, float _dt) {}
		virtual void Render(CEntityManager& _components, sf::RenderTarget& _target) {}
	};

///////////////////////////////////////////////////////////////////////////////////////
	class CSystemManager : public engine::core::NonCopyable
	{
	public:

		template<typename T, typename... Args>
		T& AddSystem(Args&&... _args)
		{
			std::unique_ptr<T> system = std::make_unique<T>(std::forward<Args>(_args)...);
			T& ref = *system;
			m_Systems.push_back(std::move(system));
			return ref;
		}

		void Frame(CEntityManager& _components, float _dt)
		{
			for (auto& s : m_Systems)
			{
				s->Frame(_components, _dt);
			}
		}

		void Render(CEntityManager& _components, sf::RenderTarget& _target)
		{
			for (auto& s : m_Systems)
			{
				s->Render(_components, _target);
			}
		}

	private:

		std::vector<std::unique_ptr<ISystem>> m_Systems;
	};
}