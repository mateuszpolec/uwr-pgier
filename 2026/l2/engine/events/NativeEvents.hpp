#pragma once

#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/System/Vector2.hpp>

#include "Event.hpp"

namespace engine::events
{
///////////////////////////////////////////////////////////////////////////////////////
	struct SWindowClosedEvent : public TEvent<SWindowClosedEvent>
	{
		SWindowClosedEvent() = default;
	};

///////////////////////////////////////////////////////////////////////////////////////
	struct SWindowResizedEvent : public TEvent<SWindowResizedEvent>
	{
		explicit SWindowResizedEvent(sf::Vector2u _size)
			: Size(_size)
		{
		}

		sf::Vector2u Size;
	};

///////////////////////////////////////////////////////////////////////////////////////
	struct SKeyPressedEvent : public TEvent<SKeyPressedEvent>
	{
		SKeyPressedEvent(sf::Keyboard::Key _code, sf::Keyboard::Scancode _scancode, bool _alt, bool _control, bool _shift, bool _system)
			: Code(_code)
			, Scancode(_scancode)
			, Alt(_alt)
			, Control(_control)
			, Shift(_shift)
			, System(_system)
		{
		}

		sf::Keyboard::Key Code;
		sf::Keyboard::Scancode Scancode;
		bool Alt;
		bool Control;
		bool Shift;
		bool System;
	};

///////////////////////////////////////////////////////////////////////////////////////
	struct SKeyReleasedEvent : public TEvent<SKeyReleasedEvent>
	{
		SKeyReleasedEvent(sf::Keyboard::Key _code, sf::Keyboard::Scancode _scancode)
			: Code(_code)
			, Scancode(_scancode)
		{
		}

		sf::Keyboard::Key Code;
		sf::Keyboard::Scancode Scancode;
	};

///////////////////////////////////////////////////////////////////////////////////////
	struct SMouseButtonPressedEvent : public TEvent<SMouseButtonPressedEvent>
	{
		SMouseButtonPressedEvent(sf::Mouse::Button _button, sf::Vector2i _position)
			: Button(_button)
			, Position(_position)
		{
		}

		sf::Mouse::Button Button;
		sf::Vector2i Position;
	};

///////////////////////////////////////////////////////////////////////////////////////
	struct SMouseButtonReleasedEvent : public TEvent<SMouseButtonReleasedEvent>
	{
		SMouseButtonReleasedEvent(sf::Mouse::Button _button, sf::Vector2i _position)
			: Button(_button)
			, Position(_position)
		{
		}

		sf::Mouse::Button Button;
		sf::Vector2i Position;
	};

///////////////////////////////////////////////////////////////////////////////////////
	struct SMouseMovedEvent : public TEvent<SMouseMovedEvent>
	{
		explicit SMouseMovedEvent(sf::Vector2i _position)
			: Position(_position)
		{
		}

		sf::Vector2i Position;
	};

///////////////////////////////////////////////////////////////////////////////////////
	struct SMouseWheelScrolledEvent : public TEvent<SMouseWheelScrolledEvent>
	{
		SMouseWheelScrolledEvent(float _delta, sf::Vector2i _position)
			: Delta(_delta)
			, Position(_position)
		{
		}

		float Delta;
		sf::Vector2i Position;
	};
}