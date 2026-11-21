#include <algorithm>
#include <utility>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include "CollisionSolver.hpp"

namespace engine::physics
{
	void ICollisionSolver::TryAddPair(std::uint32_t _a, std::uint32_t _b, const std::vector<SBody>& _bodies, std::vector<SCollisionPair>& _pairs)
	{
		const SBody& a = _bodies[_a];
		const SBody& b = _bodies[_b];

		// At least one body has to be movable
		if (a.InvMass + b.InvMass > 0.f && a.IsOnLayer(b.Layer) && a.GetBounds().findIntersection(b.GetBounds()))
		{
			_pairs.push_back({ _a, _b });
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////
	void CBruteForceSolver::FindPairs(const std::vector<SBody>& _bodies, std::vector<SCollisionPair>& _pairs)
	{
		const std::uint32_t count = static_cast<std::uint32_t>(_bodies.size());
		for (std::uint32_t i = 0; i < count; ++i)
		{
			for (std::uint32_t j = i + 1; j < count; ++j)
			{
				TryAddPair(i, j, _bodies, _pairs);
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////
	void CQuadtreeSolver::FindPairs(const std::vector<SBody>& _bodies, std::vector<SCollisionPair>& _pairs)
	{
		m_Nodes.clear();
		if (_bodies.size() < 2)
		{
			return;
		}

		sf::Vector2f min = _bodies.front().GetBounds().position;
		sf::Vector2f max = min;
		for (const SBody& body : _bodies)
		{
			const sf::FloatRect bounds = body.GetBounds();
			min = { std::min(min.x, bounds.position.x), std::min(min.y, bounds.position.y) };
			max = { std::max(max.x, bounds.position.x + bounds.size.x), std::max(max.y, bounds.position.y + bounds.size.y) };
		}

		m_Nodes.push_back(SNode{ { min, max - min } });

		for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(_bodies.size()); ++i)
		{
			Insert(i, _bodies);
		}

		CollectPairs(0, _bodies, _pairs);
	}

	void CQuadtreeSolver::RenderDebug(sf::RenderTarget& _target) const
	{
		sf::VertexArray lines(sf::PrimitiveType::Lines);

		for (const SNode& node : m_Nodes)
		{
			if (node.FirstChild >= 0)
			{
				continue;
			}

			const sf::Vector2f min = node.Bounds.position;
			const sf::Vector2f max = min + node.Bounds.size;
			const sf::Vector2f corners[] = { min, { max.x, min.y }, max, { min.x, max.y } };

			for (size_t i = 0; i < std::size(corners); ++i)
			{
				lines.append({ corners[i], DEBUG_COLOR });
				lines.append({ corners[(i + 1) % std::size(corners)], DEBUG_COLOR });
			}
		}

		_target.draw(lines);
	}

	void CQuadtreeSolver::Insert(std::uint32_t _item, const std::vector<SBody>& _bodies)
	{
		const sf::FloatRect bounds = _bodies[_item].GetBounds();
		std::int32_t node = 0;

		while (true)
		{
			if (m_Nodes[node].FirstChild < 0)
			{
				m_Nodes[node].Items.push_back(_item);
				if (m_Nodes[node].Items.size() > MAX_ITEMS && m_Nodes[node].Depth < MAX_DEPTH)
				{
					Split(node, _bodies);
				}
				return;
			}

			const std::int32_t child = FindChild(node, bounds);
			if (child < 0)
			{
				m_Nodes[node].Items.push_back(_item);
				return;
			}

			node = child;
		}
	}

	void CQuadtreeSolver::Split(std::int32_t _node, const std::vector<SBody>& _bodies)
	{
		const sf::FloatRect bounds = m_Nodes[_node].Bounds;
		const sf::Vector2f half = bounds.size * 0.5f;
		const int depth = m_Nodes[_node].Depth + 1;

		m_Nodes[_node].FirstChild = static_cast<std::int32_t>(m_Nodes.size());
		for (int i = 0; i < CHILD_COUNT; ++i)
		{
			const sf::Vector2f offset{ static_cast<float>(i % 2) * half.x, static_cast<float>(i / 2) * half.y };
			m_Nodes.push_back(SNode{ { bounds.position + offset, half }, depth });
		}

		// Items that do not fit into a single child stay in the parent
		const std::vector<std::uint32_t> items = std::move(m_Nodes[_node].Items);
		m_Nodes[_node].Items.clear();

		for (const std::uint32_t item : items)
		{
			const std::int32_t child = FindChild(_node, _bodies[item].GetBounds());
			m_Nodes[child >= 0 ? child : _node].Items.push_back(item);
		}
	}

	std::int32_t CQuadtreeSolver::FindChild(std::int32_t _node, const sf::FloatRect& _bounds) const
	{
		for (int i = 0; i < CHILD_COUNT; ++i)
		{
			const std::int32_t child = m_Nodes[_node].FirstChild + i;
			if (Contains(m_Nodes[child].Bounds, _bounds))
			{
				return child;
			}
		}
		return -1;
	}

	void CQuadtreeSolver::CollectPairs(std::int32_t _node, const std::vector<SBody>& _bodies, std::vector<SCollisionPair>& _pairs) const
	{
		const SNode& node = m_Nodes[_node];

		for (size_t i = 0; i < node.Items.size(); ++i)
		{
			for (size_t j = i + 1; j < node.Items.size(); ++j)
			{
				TryAddPair(node.Items[i], node.Items[j], _bodies, _pairs);
			}
		}

		if (node.FirstChild < 0)
		{
			return;
		}

		for (int i = 0; i < CHILD_COUNT; ++i)
		{
			for (const std::uint32_t item : node.Items)
			{
				CollectAgainstSubtree(item, node.FirstChild + i, _bodies, _pairs);
			}
			CollectPairs(node.FirstChild + i, _bodies, _pairs);
		}
	}

	void CQuadtreeSolver::CollectAgainstSubtree(std::uint32_t _item, std::int32_t _node, const std::vector<SBody>& _bodies, std::vector<SCollisionPair>& _pairs) const
	{
		const SNode& node = m_Nodes[_node];
		if (!node.Bounds.findIntersection(_bodies[_item].GetBounds()))
		{
			return;
		}

		for (const std::uint32_t other : node.Items)
		{
			TryAddPair(_item, other, _bodies, _pairs);
		}

		if (node.FirstChild < 0)
		{
			return;
		}

		for (int i = 0; i < CHILD_COUNT; ++i)
		{
			CollectAgainstSubtree(_item, node.FirstChild + i, _bodies, _pairs);
		}
	}

	bool CQuadtreeSolver::Contains(const sf::FloatRect& _outer, const sf::FloatRect& _inner)
	{
		return _inner.position.x >= _outer.position.x && _inner.position.x + _inner.size.x <= _outer.position.x + _outer.size.x
			&& _inner.position.y >= _outer.position.y && _inner.position.y + _inner.size.y <= _outer.position.y + _outer.size.y;
	}
}