#pragma once

#include <nlohmann/json.hpp>

namespace engine::ecs::components
{
///////////////////////////////////////////////////////////////////////////////////////
	enum class EDirection
	{
		Up,
		Right,
		Down,
		Left
	};

	NLOHMANN_JSON_SERIALIZE_ENUM(EDirection, 
	{
		{ EDirection::Up, "Up" },
		{ EDirection::Right, "Right" },
		{ EDirection::Down, "Down" },
		{ EDirection::Left, "Left" },
	})

	struct SCoDirection
	{
		static constexpr std::string_view JSON_NAME = "Direction";
		///////////////////////////////////////////////////////////////////////////////////////
		EDirection Direction{ EDirection::Down };
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SCoDirection, Direction)

}