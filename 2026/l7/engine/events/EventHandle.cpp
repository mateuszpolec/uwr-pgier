#include "EventHandle.hpp"
#include "EventDispatcher.hpp"

namespace engine::events
{
	CEventHandle::CEventHandle()
		: m_ListenerId(INVALID_LISTENER_ID)
	{
	}

	CEventHandle::CEventHandle(std::weak_ptr<CEventDispatcher> _dispatcher, listener_id_t _listener_id)
		: m_Dispatcher(std::move(_dispatcher))
		, m_ListenerId(_listener_id)
	{
	}

	CEventHandle::~CEventHandle()
	{
		Reset();
	}

	CEventHandle::CEventHandle(CEventHandle&& _other) noexcept
		: m_Dispatcher(std::move(_other.m_Dispatcher))
		, m_ListenerId(_other.m_ListenerId)
	{
		_other.m_Dispatcher.reset();
		_other.m_ListenerId = INVALID_LISTENER_ID;
	}

	CEventHandle& CEventHandle::operator=(CEventHandle&& _other) noexcept
	{
		if (this != &_other)
		{
			Reset();
			m_Dispatcher = std::move(_other.m_Dispatcher);
			m_ListenerId = _other.m_ListenerId;
			_other.m_Dispatcher.reset();
			_other.m_ListenerId = INVALID_LISTENER_ID;
		}

		return *this;
	}

	void CEventHandle::Reset()
	{
		if (auto dispatcher = m_Dispatcher.lock())
		{
			dispatcher->Unsubscribe(m_ListenerId);
		}

		m_Dispatcher.reset();
		m_ListenerId = INVALID_LISTENER_ID;
	}


	bool CEventHandle::IsValid() const
	{
		return !m_Dispatcher.expired() && m_ListenerId != INVALID_LISTENER_ID;
	}
}