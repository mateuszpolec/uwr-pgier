#pragma once

#include <cstdint>

#include "EventTypes.hpp"

namespace engine::events
{
/////////////////////////////////////////////////////////////
	class IEvent
	{
		public:

			virtual ~IEvent() = default;
			virtual event_id_t GetEventId() const = 0;
			virtual const char* GetEventName() const = 0;
	};

/////////////////////////////////////////////////////////////
	template<typename T>
	class TEvent : public IEvent
	{
		public:

			static constexpr event_id_t GetStaticEventId()
			{
				return typeid(T).hash_code();
			}

			event_id_t GetEventId() const override
			{
				return GetStaticEventId();
			}

			const char* GetEventName() const override
			{
				return typeid(T).name();
			}
	};
}