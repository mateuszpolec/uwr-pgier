#pragma once

#include <memory>
#include <functional>
#include <unordered_map>
#include <vector>

#include "Event.hpp"
#include "EventTypes.hpp"
#include "EventHandle.hpp"

#include "engine/core/FrameTime.hpp"

namespace engine::events
{
	class CEventDispatcher : public std::enable_shared_from_this<CEventDispatcher>
	{
		public:

			CEventDispatcher(const engine::core::SFrameTime& _frame_time);

			void Frame();

			template<typename TEvent>
			[[nodiscard]] CEventHandle Subscribe(std::function<void(const TEvent&)> _callback)
			{
				listener_id_t listener_id = ++m_NextListenerId;

				auto type_erased_callback = [callback = std::move(_callback)](const IEvent& event)
				{
					callback(static_cast<const TEvent&>(event));
				};

				m_Listeners[TEvent::GetStaticEventId()].emplace_back(SListener{ listener_id, std::move(type_erased_callback) });
				return CEventHandle(weak_from_this(), listener_id);
			}

			template<typename TEvent, typename... Args>
			void Submit(Args&&... args)
			{
				TEvent event{ std::forward<Args>(args)... };
				Dispatch(event);
			}

			template<typename TEvent, typename... Args>
			void QueueWithTimeDelay(float _delay, Args&&... args)
			{
				SQueuedEvent queued_event;
				queued_event.Event = std::make_unique<TEvent>(TEvent{ std::forward<Args>(args)... });
				
				queued_event.DispatchFrame = 0; // No frame delay, only time delay
				queued_event.DispatchTime = m_FrameTime.TimeElapsedScaled + _delay;

				m_EventQueue.push_back(std::move(queued_event));
			}

			template<typename TEvent, typename... Args>
			void QueueWithFrameDelay(uint32_t _frame_delay, Args&&... args)
			{
				SQueuedEvent queued_event;
				queued_event.Event = std::make_unique<TEvent>(TEvent{ std::forward<Args>(args)... });

				queued_event.DispatchTime = 0.0f; // No time delay, only frame delay
				queued_event.DispatchFrame = m_FrameTime.CurrentFrame + _frame_delay;

				m_EventQueue.push_back(std::move(queued_event));
			}

			void Unsubscribe(listener_id_t _listener_id);
			void Clear();

		private:

			void Dispatch(const IEvent& _event);

		private:

			struct SListener
			{
				listener_id_t ID = INVALID_LISTENER_ID;
				std::function<void(const IEvent&)> Callback;
			};

			struct SQueuedEvent
			{
				std::unique_ptr<IEvent> Event;
				uint32_t DispatchFrame = 0;
				float DispatchTime = 0.f;
			};

		private:

			const engine::core::SFrameTime& m_FrameTime;
			uint32_t m_NextListenerId = 0;

			std::unordered_map<event_id_t, std::vector<SListener>> m_Listeners;
			std::vector<SQueuedEvent> m_EventQueue;
	};
}