#include "FreeCamera.hpp"
#include "engine/game/Game.hpp"
#include "engine/events/EventDispatcher.hpp"
#include "engine/events/NativeEvents.hpp"

namespace engine::camera
{
	CFreeCamera::CFreeCamera()
	{
		engine::events::CEventDispatcher& event_dispatcher = engine::game::CGame::Get().GetEventDispatcher();

		m_KeyPressedEventHandle = event_dispatcher.Subscribe<engine::events::SKeyPressedEvent>([this](const engine::events::SKeyPressedEvent& _event)
			{
				OnKeyInput(_event.Code, true);
			});

		m_KeyReleasedEventHandle = event_dispatcher.Subscribe<engine::events::SKeyReleasedEvent>([this](const engine::events::SKeyReleasedEvent& _event)
			{
				OnKeyInput(_event.Code, false);
			});

		m_MouseWheelScrolledEventHandle = event_dispatcher.Subscribe<engine::events::SMouseWheelScrolledEvent>([this](const engine::events::SMouseWheelScrolledEvent& _event)
			{
				if (_event.Wheel != sf::Mouse::Wheel::Vertical)
				{
					return;
				}

				m_State.Zoom += _event.Delta * 0.1f;
				m_State.Zoom = std::max(0.1f, m_State.Zoom);
			});
	}

	void CFreeCamera::OnActivated(const SCameraState& _prev_state)
	{
		m_State = _prev_state;
	}

	void CFreeCamera::Frame(float _dt)
	{
		sf::Vector2f dir{ 0.f, 0.f };
		sf::Vector2f velocity{ 0.f, 0.f };

		for (size_t i = 0; i < MovementKeys.size(); ++i)
		{
			if (m_PressedKeys[i])
			{
				dir += MovementKeys[i].second;
			}
		}

		if (dir.length() > 0.f)
		{
			dir = dir.normalized();
		}

		velocity = dir * MOVE_SPEED;
		GetState().Center += velocity * _dt;
	}

	void CFreeCamera::OnKeyInput(sf::Keyboard::Key _key, bool _is_pressed)
	{
		for (size_t i = 0; i < MovementKeys.size(); ++i)
		{
			if (MovementKeys[i].first == _key)
			{
				m_PressedKeys.set(i, _is_pressed);
				return;
			}
		}
	}
}