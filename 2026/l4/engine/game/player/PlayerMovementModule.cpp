#include "PlayerMovementModule.hpp"

#include "engine/ecs/components/Component_Transform.hpp"
#include "engine/events/NativeEvents.hpp"
#include "engine/game/Game.hpp"

namespace engine::game::player
{
		CPlayerMovementModule::CPlayerMovementModule(CPlayer& _player)
		: IPlayerModule(_player)
	{
			engine::events::CEventDispatcher& event_dispatcher = CGame::Get().GetEventDispatcher();

			m_KeyPressedEventHandle = event_dispatcher.Subscribe<engine::events::SKeyPressedEvent>([this](const engine::events::SKeyPressedEvent& _event)
			{
				OnInput(_event.Code, true);
			});

			m_KeyReleasedEventHandle = event_dispatcher.Subscribe<engine::events::SKeyReleasedEvent>([this](const engine::events::SKeyReleasedEvent& _event)
			{
				OnInput(_event.Code, false);
			});

			m_WindowFocusLostEventHandle = event_dispatcher.Subscribe<engine::events::SWindowFocusLostEvent>([this](const engine::events::SWindowFocusLostEvent& _event)
			{
				m_PressedKeys.reset();
			});

	}
	
	void CPlayerMovementModule::Frame(float _dt)
	{
		sf::Vector2f dir{ 0.f, 0.f };
		for (size_t i = 0; i < MovementKeys.size(); ++i)
		{
			if (m_PressedKeys[i])
			{
				dir += MovementKeys[i].second;
			}
		}

		if (m_Direction.length() > 0.f)
		{
			dir = m_Direction.normalized();
		}

		m_Velocity = dir * MOVE_SPEED;

		engine::ecs::components::SCoTransform& co_transform = m_Player.GetComponent<engine::ecs::components::SCoTransform>();
		co_transform.Position += m_Velocity * _dt;
	}

	void CPlayerMovementModule::OnInput(sf::Keyboard::Key _input_key, bool _is_pressed)
	{
		for (size_t i = 0; i < MovementKeys.size(); ++i)
		{
			if (MovementKeys[i].first == _input_key)
			{
				m_PressedKeys.set(i, _is_pressed);
				return;
			}
		}
	}
}