#pragma once

#include <array>
#include <memory>

#include "engine/ecs/Entity.hpp"
#include "engine/ecs/EntityManager.hpp"
#include "PlayerTypes.hpp"
#include "PlayerModule.hpp"

namespace engine::game::player
{
	class CPlayer : public engine::ecs::CEntity
	{
		public:

			CPlayer(engine::ecs::CEntity _entity);

			void Frame(float _dt);

			template<typename C>
			C& GetModule()
			{
				static_assert(std::is_base_of_v<IPlayerModule, C>, "C must derive from IPlayerModule");
				return static_cast<C&>(*m_Modules[C::GetIndex()]);
			}

			template<typename C>
			const C& GetModule() const
			{
				static_assert(std::is_base_of_v<IPlayerModule, C>, "C must derive from IPlayerModule");
				return static_cast<const C&>(*m_Modules[C::GetIndex()]);
			}

		private:

			void InitializeComponents();
			void InitializeModules();

		private:

			std::array<std::unique_ptr<IPlayerModule>, PlayerModuleCount> m_Modules;
	};
}