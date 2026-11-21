#pragma once

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

#include <SFML/System/Vector2.hpp>

#include "engine/ecs/EcsTypes.hpp"
#include "engine/level/LevelTypes.hpp"

namespace engine::physics
{
	///////////////////////////////////////////////////////////////////////////////////////
	struct STraceParams
	{
		sf::Vector2f Start;
		sf::Vector2f End;
		sf::Vector2f Size{ 0.f, 0.f };
		std::vector<std::string> Layers; // Level layer names, empty = all layers
		bool TraceTiles = true;
		bool TraceEntities = true;
		engine::ecs::entity_id_t IgnoredEntity = engine::ecs::INVALID_ENTITY_ID;

		[[nodiscard]] static STraceParams FromDirection(sf::Vector2f _start, sf::Vector2f _direction, float _length)
		{
			STraceParams params;
			params.Start = _start;
			params.End = _direction.length() > 0.f ? _start + _direction.normalized() * _length : _start;
			return params;
		}

		[[nodiscard]] bool IncludesLayer(std::string_view _layer) const
		{
			return Layers.empty() || _layer.empty() || std::find(Layers.begin(), Layers.end(), _layer) != Layers.end();
		}
	};

	///////////////////////////////////////////////////////////////////////////////////////
	enum class ETraceHitType
	{
		None,
		Tile,
		Entity
	};

	struct STraceHit
	{
		ETraceHitType Type{ ETraceHitType::None };
		sf::Vector2f Point;
		sf::Vector2f Normal;
		float Distance = 0.f;
		float Fraction = 1.f; // 0..1 along Start -> End

		engine::ecs::entity_id_t Entity = engine::ecs::INVALID_ENTITY_ID; // Valid for entity hits

		sf::Vector2i Tile; // Valid for tile hits
		std::string Layer;
		engine::level::STile TileData;

		[[nodiscard]] bool IsHit() const { return Type != ETraceHitType::None; }
		[[nodiscard]] bool IsEntity() const { return Type == ETraceHitType::Entity; }
		[[nodiscard]] bool IsTile() const { return Type == ETraceHitType::Tile; }
		[[nodiscard]] explicit operator bool() const { return IsHit(); }
	};
}