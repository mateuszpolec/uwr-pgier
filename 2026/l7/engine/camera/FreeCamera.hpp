#pragma once

#include <SFML/Window/Keyboard.hpp>
#include <array>
#include <bitset>

#include "engine/ecs/EcsTypes.hpp"
#include "engine/events/EventHandle.hpp"
#include "Camera.hpp"

namespace engine::camera
{
	class CFreeCamera : public ICamera
	{
	public:

		explicit CFreeCamera();
		~CFreeCamera() override = default;

		[[nodiscard]] ECameraType GetType() const override { return ECameraType::Free; }

		void OnActivated(const SCameraState& _prev_state) override;
		void Frame(float _dt) override;

	public:

		sf::Vector2f Offset{ 0.f, 0.f };
		float DefaultZoom = 1.f;

	private:

		void OnKeyInput(sf::Keyboard::Key _key, bool _is_pressed);

	private:

		static constexpr std::array<std::pair<sf::Keyboard::Key, sf::Vector2f>, 8> MovementKeys =
		{ {
			std::make_pair(sf::Keyboard::Key::W, sf::Vector2f{ 0.f, -1.f }),
			std::make_pair(sf::Keyboard::Key::Up, sf::Vector2f{ 0.f, -1.f }),
			std::make_pair(sf::Keyboard::Key::S, sf::Vector2f{ 0.f, 1.f }),
			std::make_pair(sf::Keyboard::Key::Down, sf::Vector2f{ 0.f, 1.f }),
			std::make_pair(sf::Keyboard::Key::A, sf::Vector2f{ -1.f, 0.f }),
			std::make_pair(sf::Keyboard::Key::Left, sf::Vector2f{ -1.f, 0.f }),
			std::make_pair(sf::Keyboard::Key::D, sf::Vector2f{ 1.f, 0.f }),
			std::make_pair(sf::Keyboard::Key::Right, sf::Vector2f{ 1.f, 0.f })
		} };

		std::bitset<MovementKeys.size()> m_PressedKeys;

		engine::events::CEventHandle m_KeyPressedEventHandle;
		engine::events::CEventHandle m_KeyReleasedEventHandle;
		engine::events::CEventHandle m_MouseWheelScrolledEventHandle;

	};
}