#pragma once

#include <string_view>

#include <nlohmann/json.hpp>
#include <SFML/System/Vector2.hpp>

#include "engine/filesystem/JsonSerializationSfml.hpp"
#include "engine/physics/PhysicsTypes.hpp"

namespace engine::ecs::components
{
	struct SCoRigidBody
	{
		static constexpr std::string_view JSON_NAME = "RigidBody";
		///////////////////////////////////////////////////////////////////////////////////////
		engine::physics::EBodyType Type{ engine::physics::EBodyType::Dynamic };
		float Mass = 1.f;
		sf::Vector2f Velocity{ 0.f, 0.f };
		float Restitution = 0.f;
		float Friction = 0.2f;
		float LinearDamping = 0.f;

		///////////////////////////////////////////////////////////////////////////////////////
		[[nodiscard]] float GetInverseMass() const
		{
			return Type == engine::physics::EBodyType::Dynamic && Mass > 0.f ? 1.f / Mass : 0.f;
		}

		void ApplyImpulse(sf::Vector2f _impulse)
		{
			Velocity += _impulse * GetInverseMass();
		}
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SCoRigidBody, Type, Mass, Velocity, Restitution, Friction, LinearDamping)
}