#pragma once

#include <cstdint>
#include <string_view>

#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Mouse.hpp>

namespace engine::camera
{
	class CCameraManager;

	///////////////////////////////////////////////////////////////////////////////////////
	enum class ECameraType : uint8_t
	{
		Focus,
		Free,
		Count
	};

	[[nodiscard]] constexpr std::string_view ToString(ECameraType _type)
	{
		switch (_type)
		{
		case ECameraType::Focus: return "Focus";
		case ECameraType::Free: return "Free";
		default: return "Unknown";
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////
	struct SCameraState
	{
		sf::Vector2f Center{ 0.f, 0.f };
		float Zoom = 1.f;
	};

	///////////////////////////////////////////////////////////////////////////////////////
	class ICamera
	{
	public:

		virtual ~ICamera() = default;

		[[nodiscard]] virtual ECameraType GetType() const = 0;

		virtual void OnActivated(const SCameraState& _prev_state) {}
		virtual void OnDeactivated() {}

		virtual void Frame(float _dt) = 0;

		[[nodiscard]] const SCameraState& GetState() const { return m_State; }
		SCameraState& GetState() { return m_State; }

	protected:

		friend class CCameraManager;

		CCameraManager* m_Owner = nullptr;
		SCameraState m_State;
	};
}