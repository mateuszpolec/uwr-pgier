#pragma once

#include <cstdint>

namespace engine::events
{
	using event_id_t = size_t;
	using listener_id_t = uint32_t;

	static constexpr event_id_t INVALID_EVENT_ID = -1;
	static constexpr listener_id_t INVALID_LISTENER_ID = -1;
}
