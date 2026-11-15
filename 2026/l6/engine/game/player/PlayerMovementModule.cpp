#include "PlayerMovementModule.hpp"

#include "engine/ecs/components/Component_Transform.hpp"
#include "engine/ecs/components/Component_Direction.hpp"
#include "engine/ecs/components/Component_Animatable.hpp"

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

		if (dir.length() > 0.f)
		{
			dir = dir.normalized();
		}

		m_Velocity = dir * MOVE_SPEED;
		UpdateDirection();
		UpdateAnimation();

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
	void CPlayerMovementModule::UpdateDirection()
	{
		engine::ecs::components::SCoDirection& co_direction = m_Player.GetComponent<engine::ecs::components::SCoDirection>();

		if (m_Velocity.length() > 0.f)
		{
			if (std::abs(m_Velocity.x) > std::abs(m_Velocity.y))
			{
				co_direction.Direction = m_Velocity.x > 0.f ? engine::ecs::components::EDirection::Right : engine::ecs::components::EDirection::Left;
			}
			else
			{
				co_direction.Direction = m_Velocity.y > 0.f ? engine::ecs::components::EDirection::Down : engine::ecs::components::EDirection::Up;
			}
		}
	}
	void CPlayerMovementModule::UpdateAnimation()
	{
		static constexpr std::string_view ANIMATION_NAME = "player_move";

		const bool is_moving = m_Velocity.length() > 0.f;
		std::string anim_sequence_name;
		engine::ecs::components::SCoDirection& co_direction = m_Player.GetComponent<engine::ecs::components::SCoDirection>();

		switch(co_direction.Direction)
		{
			case engine::ecs::components::EDirection::Up:
			{
				anim_sequence_name = is_moving ? "walk_up" : "idle_up";
				break;
			}
			case engine::ecs::components::EDirection::Right:
			{
				anim_sequence_name = is_moving ? "walk_right" : "idle_right";
				break;
			}
			case engine::ecs::components::EDirection::Down:
			{
				anim_sequence_name = is_moving ? "walk_down" : "idle_down";
				break;
			}
			case engine::ecs::components::EDirection::Left:
			{
				anim_sequence_name = is_moving ? "walk_left" : "idle_left";
				break;
			}
		}

		engine::ecs::components::SCoAnimatable& co_animatable = m_Player.GetComponent<engine::ecs::components::SCoAnimatable>();
		co_animatable.Play(ANIMATION_NAME, anim_sequence_name, false);
	}
}