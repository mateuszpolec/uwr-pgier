#pragma once

#include <cstdint>
#include <string_view>
#include <variant>

#include "PhysicsTypes.hpp"
#include "engine/ecs/components/Component_RigidBody.hpp"
#include "engine/ecs/components/Component_Transform.hpp"

namespace engine::physics
{
	///////////////////////////////////////////////////////////////////////////////////////
	struct SBody
	{
		collision_shape_t Shape;
		sf::Vector2f Position;
		std::string_view Layer; // Empty = all layers

		float InvMass = 0.f;
		float Restitution = 1.f;
		float Friction = 0.f;

		engine::ecs::components::SCoTransform* Transform = nullptr;
		engine::ecs::components::SCoRigidBody* RigidBody = nullptr;

		[[nodiscard]] sf::FloatRect GetBounds() const
		{
			return std::visit([this](const auto& _shape) { return _shape.GetBounds(Position); }, Shape);
		}

		[[nodiscard]] bool IsOnLayer(std::string_view _layer) const { return Layer.empty() || _layer.empty() || Layer == _layer; }
		[[nodiscard]] sf::Vector2f GetVelocity() const { return RigidBody ? RigidBody->Velocity : sf::Vector2f{}; }

		void ApplyImpulse(sf::Vector2f _impulse)
		{
			if (InvMass > 0.f)
			{
				RigidBody->Velocity += _impulse * InvMass;
			}
		}

		void Translate(sf::Vector2f _delta)
		{
			if (InvMass > 0.f)
			{
				Transform->Position += _delta;
				Position += _delta;
			}
		}
	};

	struct SCollisionPair
	{
		std::uint32_t A = 0;
		std::uint32_t B = 0;
	};
}