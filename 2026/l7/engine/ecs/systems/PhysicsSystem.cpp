#include "PhysicsSystem.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>
#include <variant>

#include <SFML/Graphics/RectangleShape.hpp>

#include "engine/game/Game.hpp"
#include "engine/level/LevelManager.hpp"
#include "engine/game/GameVariables.hpp"

namespace engine::ecs::systems
{
	CPhysicsSystem::CPhysicsSystem()
	{
		SetSolver(m_SolverType);
	}

	void CPhysicsSystem::SetSolver(engine::physics::ECollisionSolver _type)
	{
		m_SolverType = _type;

		switch (_type)
		{
		case engine::physics::ECollisionSolver::NoCollision:
			m_Solver = std::make_unique<engine::physics::CNoCollisionSolver>();
			break;
		case engine::physics::ECollisionSolver::BruteForce:
			m_Solver = std::make_unique<engine::physics::CBruteForceSolver>();
			break;
		case engine::physics::ECollisionSolver::Quadtree:
			m_Solver = std::make_unique<engine::physics::CQuadtreeSolver>();
			break;
		}
	}

	void CPhysicsSystem::Frame(CEntityManager& _components, float _dt)
	{
		m_Accumulator = std::min(m_Accumulator + _dt, engine::game::CGameVariables::Get<float>("GamePhysicsMaxAccumulatedTime"));

		while (m_Accumulator >= FIXED_DT)
		{
			Integrate(_components);
			CollectBodies(_components);
			SolveBodies();
			SolveTiles();
			m_Accumulator -= FIXED_DT;
		}
	}

	void CPhysicsSystem::Integrate(CEntityManager& _components) const
	{
		_components.ForEach<engine::ecs::components::SCoRigidBody, engine::ecs::components::SCoTransform>([](entity_id_t, engine::ecs::components::SCoRigidBody& _rigid_body, engine::ecs::components::SCoTransform& _transform)
			{
				if (_rigid_body.Type == engine::physics::EBodyType::Static)
				{
					return;
				}

				if (_rigid_body.Type == engine::physics::EBodyType::Dynamic)
				{
					_rigid_body.Velocity *= 1.f / (1.f + _rigid_body.LinearDamping * FIXED_DT);
				}

				_transform.Position += _rigid_body.Velocity * FIXED_DT;
			});
	}

	void CPhysicsSystem::CollectBodies(CEntityManager& _components)
	{
		m_Bodies.clear();

		_components.ForEach<engine::ecs::components::SCoCollider, engine::ecs::components::SCoTransform>([this, &_components](entity_id_t _id, engine::ecs::components::SCoCollider& _collider, engine::ecs::components::SCoTransform& _transform)
		{
				m_Bodies.push_back(MakeBody(_collider, _transform, _components.FindComponent<engine::ecs::components::SCoRigidBody>(_id)));
		});
	}

	engine::physics::SBody CPhysicsSystem::MakeBody(const engine::ecs::components::SCoCollider& _collider, engine::ecs::components::SCoTransform& _transform, engine::ecs::components::SCoRigidBody* _rigid_body)
	{
		const sf::Vector2f scale{ std::abs(_transform.Scale.x), std::abs(_transform.Scale.y) };

		engine::physics::SBody body;
		body.Shape = std::visit([&scale](const auto& _shape) -> engine::physics::collision_shape_t { return _shape.Scaled(scale); }, _collider.Shape);
		body.Position = _transform.Position + _collider.Offset.componentWiseMul(scale);
		body.Layer = _collider.Layer;
		body.Transform = &_transform;

		if (_rigid_body)
		{
			body.RigidBody = _rigid_body;
			body.InvMass = _rigid_body->GetInverseMass();
			body.Restitution = _rigid_body->Restitution;
			body.Friction = _rigid_body->Friction;
		}

		return body;
	}

	void CPhysicsSystem::SolveBodies()
	{
		m_Pairs.clear();
		m_Solver->FindPairs(m_Bodies, m_Pairs);

		for (const engine::physics::SCollisionPair& pair : m_Pairs)
		{
			if (const std::optional<engine::physics::SManifold> manifold = TestBodies(m_Bodies[pair.A], m_Bodies[pair.B]))
			{
				Resolve(m_Bodies[pair.A], m_Bodies[pair.B], *manifold);
			}
		}
	}

	void CPhysicsSystem::SolveTiles()
	{
		if (m_SolverType == engine::physics::ECollisionSolver::NoCollision)
		{
			return;
		}

		const engine::level::CLevelManager& level = engine::game::CGame::Get().GetLevelManager();
		if (!level.HasLevel())
		{
			return;
		}

		engine::physics::SBody tile_body;
		tile_body.Shape = engine::physics::SAabbShape{ static_cast<sf::Vector2f>(level.GetInfo().TileSize) };

		for (engine::physics::SBody& body : m_Bodies)
		{
			if (body.InvMass <= 0.f)
			{
				continue;
			}

			const sf::FloatRect bounds = body.GetBounds();
			const sf::Vector2i min = level.WorldToTile(bounds.position);
			const sf::Vector2i max = level.WorldToTile(bounds.position + bounds.size);

			for (const engine::level::SLayerInfo& layer : level.GetInfo().Layers)
			{
				if (!body.IsOnLayer(layer.Name))
				{
					continue;
				}

				for (int y = min.y; y <= max.y; ++y)
				{
					for (int x = min.x; x <= max.x; ++x)
					{
						const sf::Vector2i tile{ x, y };
						if (!level.IsColliderAt(layer.Name, tile))
						{
							continue;
						}

						tile_body.Position = level.TileToWorld(tile);

						const std::optional<engine::physics::SManifold> manifold = TestBodies(tile_body, body);
						if (!manifold)
						{
							continue;
						}

						if (!level.IsColliderAt(layer.Name, tile + static_cast<sf::Vector2i>(manifold->Normal)))
						{
							Resolve(tile_body, body, *manifold);
						}
					}
				}
			}
		}
	}

	void CPhysicsSystem::Resolve(engine::physics::SBody& _a, engine::physics::SBody& _b, const engine::physics::SManifold& _manifold)
	{
		const float inv_mass_sum = _a.InvMass + _b.InvMass;
		if (inv_mass_sum <= 0.f)
		{
			return;
		}

		const sf::Vector2f normal = _manifold.Normal;
		const sf::Vector2f relative_velocity = _b.GetVelocity() - _a.GetVelocity();
		const float normal_velocity = relative_velocity.dot(normal);

		if (normal_velocity < 0.f)
		{
			// Normal impulse
			const float normal_impulse = -(1.f + std::min(_a.Restitution, _b.Restitution)) * normal_velocity / inv_mass_sum;
			sf::Vector2f impulse = normal * normal_impulse;

			// Friction impulse (Coulomb)
			const sf::Vector2f tangent_velocity = relative_velocity - normal * normal_velocity;
			if (tangent_velocity.lengthSquared() > std::numeric_limits<float>::epsilon())
			{
				const float friction = std::sqrt(_a.Friction * _b.Friction);
				impulse -= tangent_velocity.normalized() * std::min(tangent_velocity.length() / inv_mass_sum, normal_impulse * friction);
			}

			_a.ApplyImpulse(-impulse);
			_b.ApplyImpulse(impulse);
		}

		// Positional correction
		const sf::Vector2f correction = normal * (std::max(_manifold.Penetration - engine::game::CGameVariables::Get<float>("GamePhysicsCorrectionSlop"), 0.f) / inv_mass_sum * engine::game::CGameVariables::Get<float>("GamePhysicsCorrectionPercent"));
		_a.Translate(-correction * _a.InvMass);
		_b.Translate(correction * _b.InvMass);
	}

	///////////////////////////////////////////////////////////////////////////////////////
	std::optional<engine::physics::SManifold> CPhysicsSystem::TestBodies(const engine::physics::SBody& _a, const engine::physics::SBody& _b)
	{
		return std::visit([&_a, &_b](const auto& _shape_a, const auto& _shape_b)
			{
				return TestShapes(_shape_a, _a.Position, _shape_b, _b.Position);
			}, _a.Shape, _b.Shape);
	}

	std::optional<engine::physics::SManifold> CPhysicsSystem::TestShapes(const engine::physics::SAabbShape& _a, sf::Vector2f _position_a, const engine::physics::SAabbShape& _b, sf::Vector2f _position_b)
	{
		const sf::Vector2f delta = (_position_b + _b.Size * 0.5f) - (_position_a + _a.Size * 0.5f);
		const sf::Vector2f overlap = (_a.Size + _b.Size) * 0.5f - sf::Vector2f{ std::abs(delta.x), std::abs(delta.y) };

		if (overlap.x <= 0.f || overlap.y <= 0.f)
		{
			return std::nullopt;
		}

		if (overlap.x < overlap.y)
		{
			return engine::physics::SManifold{ { delta.x < 0.f ? -1.f : 1.f, 0.f }, overlap.x };
		}

		return engine::physics::SManifold{ { 0.f, delta.y < 0.f ? -1.f : 1.f }, overlap.y };
	}

	///////////////////////////////////////////////////////////////////////////////////////
	engine::physics::STraceHit CPhysicsSystem::LineTrace(const engine::physics::STraceParams& _params) const
	{
		engine::physics::STraceHit hit;

		if (_params.TraceTiles)
		{
			TraceTiles(_params, hit);
		}
		if (_params.TraceEntities)
		{
			TraceEntities(_params, hit);
		}

		if (hit.IsHit())
		{
			const sf::Vector2f delta = _params.End - _params.Start;
			hit.Point = _params.Start + delta * hit.Fraction;
			hit.Distance = delta.length() * hit.Fraction;
		}
		if (hit.IsTile())
		{
			hit.TileData = engine::game::CGame::Get().GetLevelManager().GetTile(hit.Layer, hit.Tile).value_or(engine::level::STile{});
		}

		return hit;
	}

	void CPhysicsSystem::TraceTiles(const engine::physics::STraceParams& _params, engine::physics::STraceHit& _hit)
	{
		constexpr float INF = std::numeric_limits<float>::infinity();

		const engine::level::CLevelManager& level = engine::game::CGame::Get().GetLevelManager();
		if (!level.HasLevel())
		{
			return;
		}

		const sf::Vector2f tile_size = static_cast<sf::Vector2f>(level.GetInfo().TileSize);
		const sf::Vector2f delta = _params.End - _params.Start;

		const sf::Vector2i reach{
			static_cast<int>(std::ceil(_params.Size.x * 0.5f / tile_size.x)),
			static_cast<int>(std::ceil(_params.Size.y * 0.5f / tile_size.y))
		};

		sf::Vector2i cell = level.WorldToTile(_params.Start);
		const sf::Vector2i step{ (delta.x > 0.f) - (delta.x < 0.f), (delta.y > 0.f) - (delta.y < 0.f) };
		const sf::Vector2f t_delta{
			step.x != 0 ? tile_size.x / std::abs(delta.x) : INF,
			step.y != 0 ? tile_size.y / std::abs(delta.y) : INF
		};
		sf::Vector2f t_next{
			step.x != 0 ? (static_cast<float>(cell.x + (step.x > 0 ? 1 : 0)) * tile_size.x - _params.Start.x) / delta.x : INF,
			step.y != 0 ? (static_cast<float>(cell.y + (step.y > 0 ? 1 : 0)) * tile_size.y - _params.Start.y) / delta.y : INF
		};

		float t_enter = 0.f;
		while (t_enter <= _hit.Fraction)
		{
			for (int y = cell.y - reach.y; y <= cell.y + reach.y; ++y)
			{
				for (int x = cell.x - reach.x; x <= cell.x + reach.x; ++x)
				{
					const sf::Vector2i tile{ x, y };

					for (const engine::level::SLayerInfo& layer : level.GetInfo().Layers)
					{
						if (_params.IncludesLayer(layer.Name) && level.IsColliderAt(layer.Name, tile) && TraceRect(_params, { level.TileToWorld(tile), tile_size }, _hit))
						{
							_hit.Type = engine::physics::ETraceHitType::Tile;
							_hit.Tile = tile;
							_hit.Layer = layer.Name;
						}
					}
				}
			}

			if (t_next.x < t_next.y)
			{
				t_enter = t_next.x;
				t_next.x += t_delta.x;
				cell.x += step.x;
			}
			else
			{
				t_enter = t_next.y;
				t_next.y += t_delta.y;
				cell.y += step.y;
			}
		}
	}

	void CPhysicsSystem::TraceEntities(const engine::physics::STraceParams& _params, engine::physics::STraceHit& _hit)
	{
		engine::game::CGame::Get().GetEntityManager().ForEach<engine::ecs::components::SCoCollider, engine::ecs::components::SCoTransform>(
			[&_params, &_hit](entity_id_t _id, engine::ecs::components::SCoCollider& _collider, engine::ecs::components::SCoTransform& _transform)
			{
				if (_id != _params.IgnoredEntity && _params.IncludesLayer(_collider.Layer) && TraceRect(_params, MakeBody(_collider, _transform, nullptr).GetBounds(), _hit))
				{
					_hit.Type = engine::physics::ETraceHitType::Entity;
					_hit.Entity = _id;
				}
			});
	}

	bool CPhysicsSystem::TraceRect(const engine::physics::STraceParams& _params, const sf::FloatRect& _rect, engine::physics::STraceHit& _hit)
	{
		const sf::Vector2f half = _params.Size * 0.5f;
		const sf::Vector2f delta = _params.End - _params.Start;

		const float origin[2] = { _params.Start.x, _params.Start.y };
		const float direction[2] = { delta.x, delta.y };
		const float min[2] = { _rect.position.x - half.x, _rect.position.y - half.y };
		const float max[2] = { _rect.position.x + _rect.size.x + half.x, _rect.position.y + _rect.size.y + half.y };

		float t_min = 0.f;
		float t_max = 1.f;
		sf::Vector2f normal;

		for (int axis = 0; axis < 2; ++axis)
		{
			if (std::abs(direction[axis]) < std::numeric_limits<float>::epsilon())
			{
				if (origin[axis] < min[axis] || origin[axis] > max[axis])
				{
					return false;
				}
				continue;
			}

			float t_near = (min[axis] - origin[axis]) / direction[axis];
			float t_far = (max[axis] - origin[axis]) / direction[axis];
			if (t_near > t_far)
			{
				std::swap(t_near, t_far);
			}

			if (t_near > t_min)
			{
				const float sign = direction[axis] > 0.f ? -1.f : 1.f;
				t_min = t_near;
				normal = axis == 0 ? sf::Vector2f{ sign, 0.f } : sf::Vector2f{ 0.f, sign };
			}

			t_max = std::min(t_max, t_far);
			if (t_min > t_max)
			{
				return false;
			}
		}

		if (_hit.IsHit() && t_min >= _hit.Fraction)
		{
			return false;
		}

		_hit.Fraction = t_min;
		_hit.Normal = normal;
		return true;
	}

	///////////////////////////////////////////////////////////////////////////////////////
	void CPhysicsSystem::RenderDebug(sf::RenderTarget& _target) const
	{
		if (DebugDrawSolver)
		{
			m_Solver->RenderDebug(_target);
		}

		if (!DebugDraw)
		{
			return;
		}

		sf::RectangleShape shape;
		shape.setFillColor(sf::Color::Transparent);
		shape.setOutlineColor(sf::Color::Green);
		shape.setOutlineThickness(-1.f);

		engine::game::CGame::Get().GetEntityManager().ForEach<engine::ecs::components::SCoCollider, engine::ecs::components::SCoTransform>(
			[&shape, &_target](entity_id_t, engine::ecs::components::SCoCollider& _collider, engine::ecs::components::SCoTransform& _transform)
			{
				const sf::FloatRect bounds = MakeBody(_collider, _transform, nullptr).GetBounds();
				shape.setPosition(bounds.position);
				shape.setSize(bounds.size);
				_target.draw(shape);
			});
	}
}