#include <cmath>

#include "FocusCamera.hpp"
#include "engine/ecs/components/Component_Transform.hpp"
#include "engine/ecs/components/Component_Renderable.hpp"
#include "engine/game/Game.hpp"

namespace engine::camera
{
	CFocusCamera::CFocusCamera(std::optional<engine::ecs::entity_id_t> _target)
		: m_Target(_target)
	{
	}

	void CFocusCamera::OnActivated(const SCameraState& _previous)
	{
		m_State.Center = _previous.Center;
		m_State.Zoom = KeepPreviousZoom ? _previous.Zoom : DefaultZoom;

		if (SnapOnActivate)
		{
			SnapToTarget();
		}
	}

	void CFocusCamera::Frame(float _dt, bool _keyboard_enabled)
	{
		const std::optional<sf::Vector2f> target = GetTargetPosition();
		if (!target)
		{
			return;
		}

		if (Smoothing <= 0.f)
		{
			m_State.Center = *target;
			return;
		}

		const float t = 1.f - std::exp(-Smoothing * _dt);
		m_State.Center += (*target - m_State.Center) * t;
	}

	void CFocusCamera::SnapToTarget()
	{
		if (const std::optional<sf::Vector2f> target = GetTargetPosition())
		{
			m_State.Center = *target;
		}
	}

	std::optional<sf::Vector2f> CFocusCamera::GetTargetPosition() const
	{
		if (!m_Target)
		{
			return std::nullopt;
		}

		const engine::ecs::CEntityManager& entities = engine::game::CGame::Get().GetEntityManager();
		if (!entities.IsValid(*m_Target))
		{
			return std::nullopt;
		}

		const engine::ecs::components::SCoTransform* transform = entities.FindComponent<engine::ecs::components::SCoTransform>(*m_Target);
		if (!transform)
		{
			return std::nullopt;
		}

		sf::Vector2f position = transform->Position;

		const engine::ecs::components::SCoRenderable* renderable = entities.FindComponent<engine::ecs::components::SCoRenderable>(*m_Target);
		if (!renderable)
		{
			return std::nullopt;
		}

		const sf::FloatRect bounds = renderable->GetLocalBounds();
		const sf::Vector2f local_center = bounds.position + bounds.size / 2.f;
		position += { local_center.x* transform->Scale.x, local_center.y* transform->Scale.y };

		return position + Offset;
	}
}