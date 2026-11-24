#pragma once

#include <memory>
#include <optional>
#include <vector>

#include <SFML/Graphics/RenderTarget.hpp>

#include "engine/ecs/SystemManager.hpp"
#include "engine/ecs/components/Component_Collider.hpp"
#include "engine/physics/CollisionSolver.hpp"
#include "engine/physics/Trace.hpp"

namespace engine::ecs::systems
{
	class CPhysicsSystem : public engine::ecs::ISystem
	{
	public:

		CPhysicsSystem();

		void Frame(CEntityManager& _components, float _dt) override;
		void RenderDebug(sf::RenderTarget& _target) const;
		ESystemType GetType() const override { return ESystemType::Physics; }

		void SetSolver(engine::physics::ECollisionSolver _type);
		[[nodiscard]] engine::physics::ECollisionSolver GetSolverType() const { return m_SolverType; }
		[[nodiscard]] engine::physics::STraceHit LineTrace(const engine::physics::STraceParams& _params) const;

	public:

		static constexpr float FIXED_DT = 1.f / 60.f; // [s]

		bool DebugDraw = false;
		bool DebugDrawSolver = false;

	private:

		void Integrate(CEntityManager& _components) const;
		void CollectBodies(CEntityManager& _components);
		void CollectLevelBodies();
		void SolveBodies();

		[[nodiscard]] static engine::physics::SBody MakeBody(const components::SCoCollider& _collider, components::SCoTransform& _transform, components::SCoRigidBody* _rigid_body);
		static void Resolve(engine::physics::SBody& _a, engine::physics::SBody& _b, const engine::physics::SManifold& _manifold);

		// Narrow phase
		[[nodiscard]] static std::optional<engine::physics::SManifold> TestBodies(const engine::physics::SBody& _a, const engine::physics::SBody& _b);
		[[nodiscard]] static std::optional<engine::physics::SManifold> TestShapes(const engine::physics::SAabbShape& _a, sf::Vector2f _position_a, const engine::physics::SAabbShape& _b, sf::Vector2f _position_b);

		// Trace
		static void TraceTiles(const engine::physics::STraceParams& _params, engine::physics::STraceHit& _hit);
		static void TraceEntities(const engine::physics::STraceParams& _params, engine::physics::STraceHit& _hit);
		static bool TraceRect(const engine::physics::STraceParams& _params, const sf::FloatRect& _rect, engine::physics::STraceHit& _hit);

	private:

		float m_Accumulator = 0.f;

		engine::physics::ECollisionSolver m_SolverType{ engine::physics::ECollisionSolver::Quadtree };
		std::unique_ptr<engine::physics::ICollisionSolver> m_Solver;

		std::vector<engine::physics::SBody> m_Bodies;
		std::vector<engine::physics::SCollisionPair> m_Pairs;
	};
}