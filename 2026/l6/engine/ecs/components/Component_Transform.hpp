#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/System/Angle.hpp>

#include "engine/filesystem/JsonSerializationSfml.hpp"

namespace engine::ecs::components
{
	struct SCoTransform
	{
		static constexpr std::string_view JSON_NAME = "Transform";

		sf::Vector2f Position{ 0.f, 0.f };
		sf::Vector2f Scale{ 1.f, 1.f };
		sf::Angle Rotation = sf::degrees(0.f);
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SCoTransform, Position, Scale, Rotation)
}