#pragma once

#include <string_view>
#include <variant>

#include <nlohmann/json.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

#include "engine/filesystem/JsonSerializationSfml.hpp"

namespace engine::physics
{
	///////////////////////////////////////////////////////////////////////////////////////
	enum class EBodyType
	{
		Static,
		Dynamic,
		Kinematic
	};
	NLOHMANN_JSON_SERIALIZE_ENUM(EBodyType, {
		{ EBodyType::Static, "Static" },
		{ EBodyType::Dynamic, "Dynamic" },
		{ EBodyType::Kinematic, "Kinematic" }
		})

		///////////////////////////////////////////////////////////////////////////////////////
	struct SAabbShape
	{
		static constexpr std::string_view JSON_NAME = "AABB";

		sf::Vector2f Size{ 32.f, 32.f };

		[[nodiscard]] sf::FloatRect GetBounds(sf::Vector2f _position) const { return { _position, Size }; }
		[[nodiscard]] SAabbShape Scaled(sf::Vector2f _scale) const { return { Size.componentWiseMul(_scale) }; }
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SAabbShape, Size)

	// Every shape provides JSON_NAME, GetBounds() and Scaled()
	using collision_shape_t = std::variant<SAabbShape>;

	///////////////////////////////////////////////////////////////////////////////////////
		// Normal points from A to B
	struct SManifold
	{
		sf::Vector2f Normal;
		float Penetration = 0.f;
	};
}