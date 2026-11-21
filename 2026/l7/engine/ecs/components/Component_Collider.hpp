#pragma once

#include <string>
#include <string_view>

#include <nlohmann/json.hpp>
#include <SFML/System/Vector2.hpp>

#include "engine/physics/PhysicsTypes.hpp"

namespace engine::ecs::components
{
	struct SCoCollider
	{
		static constexpr std::string_view JSON_NAME = "Collider";
		///////////////////////////////////////////////////////////////////////////////////////
		engine::physics::collision_shape_t Shape;
		sf::Vector2f Offset{ 0.f, 0.f };
		std::string Layer; // Level layer name, empty = all layers
	};

	void from_json(const nlohmann::json& _j, SCoCollider& _component);
}