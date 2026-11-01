#pragma once

#include "engine/ecs/SystemManager.hpp"

namespace engine::ecs::systems
{
	class CRenderSystem : public engine::ecs::ISystem
	{
		public:

			void Render(CEntityManager& _components, sf::RenderTarget& _target) override;
	};
}