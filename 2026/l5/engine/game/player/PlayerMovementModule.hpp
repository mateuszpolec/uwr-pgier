#pragma once

#include <array>
#include <bitset>
#include <utility>

#include <SFML/Window/Keyboard.hpp>
#include <SFML/System/Vector2.hpp>

#include "PlayerModule.hpp"
#include "PlayerTypes.hpp"
#include "engine/events/EventHandle.hpp"

namespace engine::game::player
{
	class CPlayerMovementModule : public IPlayerModule
	{
	public:

		CPlayerMovementModule(CPlayer& player);

		void Frame(float _dt) override;

		static EPlayerModule GetIndex()
		{
			return EPlayerModule::Movement;
		}

	private:

		void OnInput(sf::Keyboard::Key _input_key, bool _is_pressed);

		void UpdateDirection();
		void UpdateAnimation();

	private:

		static constexpr float MOVE_SPEED = 200.f; // [px/s]

		static constexpr std::array<std::pair<sf::Keyboard::Key, sf::Vector2f>, 8> MovementKeys =
		{{
			std::make_pair(sf::Keyboard::Key::W, sf::Vector2f{ 0.f, -1.f }),
			std::make_pair(sf::Keyboard::Key::Up, sf::Vector2f{ 0.f, -1.f }),
			std::make_pair(sf::Keyboard::Key::S, sf::Vector2f{ 0.f, 1.f }),
			std::make_pair(sf::Keyboard::Key::Down, sf::Vector2f{ 0.f, 1.f }),
			std::make_pair(sf::Keyboard::Key::A, sf::Vector2f{ -1.f, 0.f }),
			std::make_pair(sf::Keyboard::Key::Left, sf::Vector2f{ -1.f, 0.f }),
			std::make_pair(sf::Keyboard::Key::D, sf::Vector2f{ 1.f, 0.f }),
			std::make_pair(sf::Keyboard::Key::Right, sf::Vector2f{ 1.f, 0.f })
		} };

		engine::events::CEventHandle m_KeyPressedEventHandle;
		engine::events::CEventHandle m_KeyReleasedEventHandle;
		engine::events::CEventHandle m_WindowFocusLostEventHandle;

		std::bitset<MovementKeys.size()> m_PressedKeys;

		sf::Vector2f m_Velocity;
	};
}