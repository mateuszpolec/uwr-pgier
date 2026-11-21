#include <algorithm>
#include <cmath>

#include "CameraManager.hpp"
#include "engine/core/Logger.hpp"
#include "engine/events/EventDispatcher.hpp"
#include "engine/events/NativeEvents.hpp"
#include "engine/game/Game.hpp"

namespace engine::camera
{
	CCameraManager::CCameraManager(sf::Vector2u _viewport_size)
		: m_ViewportSize(_viewport_size)
	{
		m_Stack.reserve(4);

		m_ResizeEventHandle = engine::game::CGame::Get().GetEventDispatcher().Subscribe<engine::events::SWindowResizedEvent>(
			[this](const engine::events::SWindowResizedEvent& _event)
			{
				SetViewportSize(_event.Size);
			});

		RefreshView();
	}

	ICamera* CCameraManager::Find(ECameraType _type) const
	{
		const size_t index = static_cast<size_t>(_type);
		return index < m_Cameras.size() ? m_Cameras[index].get() : nullptr;
	}

	void CCameraManager::Push(ECameraType _type)
	{
		ICamera* camera = Find(_type);
		ASSERT(camera != nullptr, "Camera is not registered");
		ASSERT(std::find(m_Stack.begin(), m_Stack.end(), _type) == m_Stack.end(), "Camera is already on the stack");

		ICamera* previous = HasActive() ? &GetActive() : nullptr;
		m_Stack.push_back(_type);
		SwitchActive(previous, camera);

		LOG_INFO("Camera pushed: {} (stack depth {})", ToString(_type), m_Stack.size());
	}

	void CCameraManager::Pop()
	{
		if (m_Stack.size() <= 1)
		{
			LOG_WARN("Cannot pop the base camera");
			return;
		}

		ICamera* previous = &GetActive();
		const ECameraType popped = m_Stack.back();
		m_Stack.pop_back();
		SwitchActive(previous, &GetActive());

		LOG_INFO("Camera popped: {} -> {}", ToString(popped), ToString(GetActiveType()));
	}

	void CCameraManager::Replace(ECameraType _type)
	{
		if (m_Stack.empty())
		{
			Push(_type);
			return;
		}

		ICamera* camera = Find(_type);
		ASSERT(camera != nullptr, "Camera is not registered");
		ASSERT(std::find(m_Stack.begin(), m_Stack.end() - 1, _type) == m_Stack.end() - 1, "Camera is already on the stack");

		ICamera* previous = &GetActive();
		m_Stack.back() = _type;
		SwitchActive(previous, camera);
	}

	ECameraType CCameraManager::GetActiveType() const
	{
		ASSERT(HasActive(), "No active camera");
		return m_Stack.back();
	}

	ICamera& CCameraManager::GetActive() const
	{
		ICamera* camera = Find(GetActiveType());
		ASSERT(camera != nullptr, "Active camera is not registered");
		return *camera;
	}

	bool CCameraManager::IsActive(ECameraType _type) const
	{
		return HasActive() && m_Stack.back() == _type;
	}

	void CCameraManager::SwitchActive(ICamera* _from, ICamera* _to)
	{
		if (_from == _to)
		{
			return;
		}

		const SCameraState previous_state = _from ? _from->GetState() : SCameraState{};

		if (_from)
		{
			_from->OnDeactivated();
		}
		if (_to)
		{
			_to->OnActivated(previous_state);
		}

		m_Velocity = { 0.f, 0.f };
		RefreshView();
	}

	void CCameraManager::Frame(float _dt)
	{
		if (!HasActive())
		{
			return;
		}

		ICamera& camera = GetActive();
		const sf::Vector2f previous_center = camera.GetState().Center;

		camera.Frame(_dt);

		SCameraState& state = camera.GetState();
		state.Zoom = std::max(state.Zoom, 0.0001f);

		m_Velocity = _dt > 0.f ? (state.Center - previous_center) / _dt : sf::Vector2f{ 0.f, 0.f };
		RefreshView();
	}


	void CCameraManager::SetViewportSize(sf::Vector2u _size)
	{
		m_ViewportSize = _size;
		RefreshView();
	}

	void CCameraManager::RefreshView()
	{
		const SCameraState state = HasActive() ? GetActive().GetState() : SCameraState{};
		const sf::Vector2f viewport = static_cast<sf::Vector2f>(m_ViewportSize);

		m_View.setCenter(state.Center);
		m_View.setSize(viewport / state.Zoom);
		m_View.setRotation(sf::degrees(0.f));
	}

	sf::Vector2f CCameraManager::ScreenToWorld(sf::Vector2i _pixel) const
	{
		const SCameraState state = HasActive() ? GetActive().GetState() : SCameraState{};
		const sf::Vector2f half_viewport = static_cast<sf::Vector2f>(m_ViewportSize) / 2.f;

		return state.Center + (static_cast<sf::Vector2f>(_pixel) - half_viewport) / state.Zoom;
	}

	sf::Vector2i CCameraManager::WorldToScreen(sf::Vector2f _world) const
	{
		const SCameraState state = HasActive() ? GetActive().GetState() : SCameraState{};
		const sf::Vector2f half_viewport = static_cast<sf::Vector2f>(m_ViewportSize) / 2.f;

		const sf::Vector2f pixel = (_world - state.Center) * state.Zoom + half_viewport;
		return { static_cast<int>(std::floor(pixel.x)), static_cast<int>(std::floor(pixel.y)) };
	}

	sf::FloatRect CCameraManager::GetVisibleWorldRect() const
	{
		const sf::Vector2f size = m_View.getSize();
		return { m_View.getCenter() - size / 2.f, size };
	}

	sf::FloatRect CCameraManager::GetStreamingWorldRect() const
	{
		sf::FloatRect rect = GetVisibleWorldRect();
		const sf::Vector2f lookahead = m_Velocity * StreamingLookahead;

		if (lookahead.x < 0.f)
		{
			rect.position.x += lookahead.x;
		}
		if (lookahead.y < 0.f)
		{
			rect.position.y += lookahead.y;
		}
		rect.size.x += std::abs(lookahead.x);
		rect.size.y += std::abs(lookahead.y);

		return rect;
	}
}