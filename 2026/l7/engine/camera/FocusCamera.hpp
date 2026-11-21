#pragma once

#include <optional>

#include "engine/ecs/EcsTypes.hpp"
#include "Camera.hpp"

namespace engine::camera
{
	class CFocusCamera : public ICamera
	{
	public:

		explicit CFocusCamera(std::optional<engine::ecs::entity_id_t> _target = std::nullopt);
		~CFocusCamera() override = default;

		[[nodiscard]] ECameraType GetType() const override { return ECameraType::Focus; }

		void OnActivated(const SCameraState& _prev_state) override;
		void Frame(float _dt) override;

		void SetTarget(std::optional<engine::ecs::entity_id_t> _target) { m_Target = _target; }
		[[nodiscard]] std::optional<engine::ecs::entity_id_t> GetTarget() const { return m_Target; }

		void SnapToTarget();

	public:

		float Smoothing = 10.f;
		sf::Vector2f Offset{ 0.f, 0.f };
		bool SnapOnActivate = false;
		float DefaultZoom = 1.f;

	private:

		[[nodiscard]] std::optional<sf::Vector2f> GetTargetPosition() const;

	private:

		std::optional<engine::ecs::entity_id_t> m_Target;
	};
}