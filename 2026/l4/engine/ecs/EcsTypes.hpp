#pragma once

#include <cstdint>

namespace engine::ecs
{
	using entity_id_t = std::uint32_t;
	static constexpr entity_id_t INVALID_ENTITY_ID = static_cast<entity_id_t>(-1);
}