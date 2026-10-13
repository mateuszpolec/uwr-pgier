#pragma once

#include <memory>
#include <cstdint>

#include "EventTypes.hpp"
#include "engine/core/NonCopyable.hpp"

namespace engine::events
{
	class CEventDispatcher;

	class CEventHandle : public engine::core::NonCopyable
	{
		public:

			CEventHandle();
			CEventHandle(std::weak_ptr<CEventDispatcher> _dispatcher, listener_id_t _listener_id);
			~CEventHandle();

			CEventHandle(CEventHandle&& _other) noexcept;
			CEventHandle& operator=(CEventHandle&& _other) noexcept;

			void Reset();
			[[nodiscard]] bool IsValid() const;

		private:

			std::weak_ptr<CEventDispatcher> m_Dispatcher;
			listener_id_t m_ListenerId;
	};
}