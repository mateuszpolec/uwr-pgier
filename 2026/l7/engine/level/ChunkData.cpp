#include "ChunkData.hpp"

namespace engine::level
{
	void SLayerData::SetDefaultSize()
	{
		Tiles.resize(CHUNK_AREA, engine::assets::INVALID_TILE_INDEX);
	}

	STile SLayerData::Get(int _local_index) const
	{
		// First 16 bits are tile index, next 16 bits are tileset index
		const int32_t tile_data = Tiles[_local_index];

		if (tile_data == engine::assets::INVALID_TILE_INDEX)
		{
			return STile();
		}

		// Extract the tileset index and tile index from the packed integer
		const int32_t tileset_index = (tile_data >> TILESET_SHIFT);
		if (tileset_index < 0 || tileset_index >= static_cast<int32_t>(Tilesets.size()))
		{
			return STile();
		}

		return STile( Tilesets[tileset_index],	tile_data & INDEX_MASK);
	}

	void SLayerData::Set(int _local_index, const STile& _tile)
	{
		if (!_tile.IsValid())
		{
			Tiles[_local_index] = engine::assets::INVALID_TILE_INDEX;
			return;
		}

		auto it = std::find(Tilesets.begin(), Tilesets.end(), _tile.Tileset);
		if (it == Tilesets.end())
		{
			Tilesets.push_back(_tile.Tileset);
			it = std::prev(Tilesets.end());
		}

		const int32_t tileset_index = static_cast<int32_t>(std::distance(Tilesets.begin(), it));
		Tiles[_local_index] = (tileset_index << TILESET_SHIFT) | (_tile.Index & INDEX_MASK);
	}

	bool SLayerData::IsEmptyTileAt(int _local_index) const
	{
		return Tiles[_local_index] == engine::assets::INVALID_TILE_INDEX;
	}

	int SLayerData::CountTiles() const
	{
		return static_cast<int>(std::count_if(Tiles.begin(), Tiles.end(), [](int32_t tile) { return tile != engine::assets::INVALID_TILE_INDEX; }));
	}

	int SLayerData::CountColliders() const
	{
		return CollisionRects.size();
	}

	SLayerData* SChunkData::FindLayer(std::string_view _name)
	{
		auto it = std::find_if(Layers.begin(), Layers.end(), [_name](const SLayerData& layer) { return layer.Name == _name; });
		if (it != Layers.end())
		{
			return &(*it);
		}

		return nullptr;
	}
	const SLayerData* SChunkData::FindLayer(std::string_view _name) const
	{
		auto it = std::find_if(Layers.begin(), Layers.end(), [_name](const SLayerData& layer) { return layer.Name == _name; });
		if (it != Layers.end())
		{
			return &(*it);
		}

		return nullptr;
	}
}