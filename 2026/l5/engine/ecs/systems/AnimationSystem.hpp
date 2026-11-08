#pragma once

#include "engine/ecs/SystemManager.hpp"

namespace engine::ecs::systems
{
	class CAnimationSystem : public engine::ecs::ISystem
	{
	public:

		void Frame(CEntityManager& _components, float _dt) override;
	};
}