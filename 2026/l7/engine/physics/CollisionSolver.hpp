#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

#include "PhysicsBody.hpp"

namespace engine::physics
{
	///////////////////////////////////////////////////////////////////////////////////////
	enum class ECollisionSolver
	{
		NoCollision,
		BruteForce,
		Quadtree
	};

	[[nodiscard]] constexpr std::string_view ToString(ECollisionSolver _type)
	{
		switch (_type)
		{
		case ECollisionSolver::NoCollision:
			return "NoCollision";
		case ECollisionSolver::BruteForce:
			return "BruteForce";
		case ECollisionSolver::Quadtree:
			return "Quadtree";
		}
		return "Unknown";
	}

	///////////////////////////////////////////////////////////////////////////////////////
	class ICollisionSolver
	{
	public:

		virtual ~ICollisionSolver() = default;
		virtual void FindPairs(const std::vector<SBody>& _bodies, std::vector<SCollisionPair>& _pairs) = 0;
		virtual void RenderDebug(sf::RenderTarget& _target) const {}

	protected:

		static void TryAddPair(std::uint32_t _a, std::uint32_t _b, const std::vector<SBody>& _bodies, std::vector<SCollisionPair>& _pairs);
	};

	///////////////////////////////////////////////////////////////////////////////////////
	class CNoCollisionSolver : public ICollisionSolver
	{
	public:

		void FindPairs(const std::vector<SBody>& _bodies, std::vector<SCollisionPair>& _pairs) override {}
	};

	///////////////////////////////////////////////////////////////////////////////////////
	class CBruteForceSolver : public ICollisionSolver
	{
	public:

		void FindPairs(const std::vector<SBody>& _bodies, std::vector<SCollisionPair>& _pairs) override;
	};

	///////////////////////////////////////////////////////////////////////////////////////
	class CQuadtreeSolver : public ICollisionSolver
	{
	public:

		void FindPairs(const std::vector<SBody>& _bodies, std::vector<SCollisionPair>& _pairs) override;
		void RenderDebug(sf::RenderTarget& _target) const override;

	private:

		struct SNode
		{
			sf::FloatRect Bounds;
			int Depth = 0;
			std::int32_t FirstChild = -1;
			std::vector<std::uint32_t> Items;
		};

		void Insert(std::uint32_t _item, const std::vector<SBody>& _bodies);
		void Split(std::int32_t _node, const std::vector<SBody>& _bodies);
		[[nodiscard]] std::int32_t FindChild(std::int32_t _node, const sf::FloatRect& _bounds) const;

		void CollectPairs(std::int32_t _node, const std::vector<SBody>& _bodies, std::vector<SCollisionPair>& _pairs) const;
		void CollectAgainstSubtree(std::uint32_t _item, std::int32_t _node, const std::vector<SBody>& _bodies, std::vector<SCollisionPair>& _pairs) const;

		[[nodiscard]] static bool Contains(const sf::FloatRect& _outer, const sf::FloatRect& _inner);

	private:

		static constexpr sf::Color DEBUG_COLOR{ 0, 200, 255, 160 };
		std::vector<SNode> m_Nodes;
	};
}