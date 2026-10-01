#include "EventDispatcher.hpp"

namespace engine::events
{
	CEventDispatcher::CEventDispatcher(const engine::core::SFrameTime& _frame_time)
		: m_FrameTime(_frame_time)
		, m_NextListenerId(0)
	{
	}

	void CEventDispatcher::Frame()
	{
		auto it = m_EventQueue.begin();
		while (it != m_EventQueue.end())
		{
			bool should_dispatch = false;
			if (it->DispatchFrame > 0 && it->DispatchFrame <= m_FrameTime.CurrentFrame)
			{
				should_dispatch = true;
			}

			else if (it->DispatchTime > 0.0f && it->DispatchTime <= m_FrameTime.TimeElapsedScaled)
			{
				should_dispatch = true;
			}

			if (should_dispatch)
			{
				Dispatch(*it->Event);
				it = m_EventQueue.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	void CEventDispatcher::Unsubscribe(listener_id_t _listener_id)
	{
		for (auto& [event_id, listeners] : m_Listeners)
		{
			auto it = std::remove_if(listeners.begin(), listeners.end(),
				[_listener_id](const SListener& listener) { return listener.ID == _listener_id; });
			if (it != listeners.end())
			{
				listeners.erase(it, listeners.end());
				break;
			}
		}
	}

	void CEventDispatcher::Clear()
	{
		m_Listeners.clear();
		m_EventQueue.clear();
	}

	void CEventDispatcher::Dispatch(const IEvent& _event)
	{
		auto it = m_Listeners.find(_event.GetEventId());
		if (it != m_Listeners.end())
		{
			for (const auto& listener : it->second)
			{
				listener.Callback(_event);
			}
		}
	}
}
