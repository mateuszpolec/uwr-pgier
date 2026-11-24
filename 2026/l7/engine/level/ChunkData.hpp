#pragma once

#include <nlohmann/json.hpp>
#include <cstdint>
#include <vector>

#include "LevelTypes.hpp"
#include "engine/assets/Tileset.hpp"

namespace engine::level
{
///////////////////////////////////////////////////////////////////////////////////////
	struct SPrefabInstanceData
	{
		std::string Prefab;
		std::string Preset;
		sf::Vector2f Position{ 0.f, 0.f };
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SPrefabInstanceData, Prefab, Preset, Position)

	///////////////////////////////////////////////////////////////////////////////////////
	struct SCollisionRectData
	{
		sf::Vector2f Position{ 0.f, 0.f }; // Relative to the chunk origin
		sf::Vector2f Size{ 0.f, 0.f };
		float Friction = 0.f;

		[[nodiscard]] sf::FloatRect GetWorldBounds(sf::Vector2f _chunk_origin) const { return { _chunk_origin + Position, Size }; }
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SCollisionRectData, Position, Size, Friction)

///////////////////////////////////////////////////////////////////////////////////////
	struct SLayerData
	{
		static constexpr int32_t TILESET_SHIFT = 16;
		static constexpr int32_t INDEX_MASK = 0xFFFF;

		std::string Name;
		ELayerType Type{ ELayerType::Tiles };
		bool Visible = true;

		std::vector<std::string> Tilesets;
		std::vector<engine::assets::tile_index_t> Tiles = std::vector<engine::assets::tile_index_t>(CHUNK_AREA, engine::assets::INVALID_TILE_INDEX);
		std::vector<SCollisionRectData> CollisionRects;
		std::vector<SPrefabInstanceData> Prefabs;

		void SetDefaultSize();

		[[nodiscard]] STile Get(int _local_index) const;
		void Set(int _local_index, const STile& _tile);

		[[nodiscard]] bool IsEmptyTileAt(int _local_index) const;
		[[nodiscard]] int CountTiles() const;
		[[nodiscard]] int CountColliders() const;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SLayerData, Name, Type, Visible, Tilesets, Tiles, CollisionRects)

///////////////////////////////////////////////////////////////////////////////////////
	struct SChunkData
	{
		static constexpr std::string_view EXTENSION = ".chunk";

		SChunkCoordinates Coordinates;
		std::vector<SLayerData> Layers;

		[[nodiscard]] SLayerData* FindLayer(std::string_view _name);
		[[nodiscard]] const SLayerData* FindLayer(std::string_view _name) const;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SChunkData, Coordinates, Layers)
}