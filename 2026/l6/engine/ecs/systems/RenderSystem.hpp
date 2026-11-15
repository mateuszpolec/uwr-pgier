#pragma once

#include "engine/ecs/SystemManager.hpp"

namespace engine::ecs::systems
{
	class CRenderSystem : public engine::ecs::ISystem
	{
		public:

			void Frame(CEntityManager& _components, float _dt) override;
			void Render(CEntityManager& _components, engine::render::CRenderQueue& _render_queue) override;
	};
}