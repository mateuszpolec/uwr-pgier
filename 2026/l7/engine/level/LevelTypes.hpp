#pragma once

#include <cstdint>
#include <nlohmann/json.hpp>

#include "engine/assets/Tileset.hpp"

namespace engine::level
{
	static constexpr int CHUNK_SIZE = 32;
	static constexpr int CHUNK_AREA = CHUNK_SIZE * CHUNK_SIZE;

///////////////////////////////////////////////////////////////////////////////////////
	struct SChunkCoordinates
	{
		int X{ 0 };
		int Y{ 0 };

		bool operator==(const SChunkCoordinates&) const = default;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SChunkCoordinates, X, Y)

	struct SChunkCoordinatesHash
	{
		size_t operator()(const SChunkCoordinates& _coord) const noexcept
		{
			const uint64_t key = (static_cast<uint64_t>(static_cast<uint32_t>(_coord.X)) << 32) | static_cast<uint32_t>(_coord.Y);
			return std::hash<uint64_t>{}(key);
		}
	};

///////////////////////////////////////////////////////////////////////////////////////
	struct STile
	{
		std::string Tileset;
		engine::assets::tile_index_t Index{ engine::assets::INVALID_TILE_INDEX };

		[[nodiscard]] bool IsValid() const { return !Tileset.empty() && Index != engine::assets::INVALID_TILE_INDEX; }
		[[nodiscard]] bool operator==(const STile&) const = default;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(STile, Tileset, Index)

	///////////////////////////////////////////////////////////////////////////////////////
	enum class ELayerType
	{
		Tiles,
		Objects
	};
	NLOHMANN_JSON_SERIALIZE_ENUM(ELayerType, {
		{ ELayerType::Tiles, "Tiles" },
		{ ELayerType::Objects, "Objects" }
		})

	[[nodiscard]] constexpr std::string_view ToString(ELayerType _type)
	{
		switch (_type)
		{
		case ELayerType::Tiles:
			return "Tiles";
		case ELayerType::Objects:
			return "Objects";
		}
		return "Unknown";
	}
}