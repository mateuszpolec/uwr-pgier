#pragma once

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include "engine/core/NonCopyable.hpp"
#include "LevelData.hpp"
#include "Chunk.hpp"
#include "engine/render/RenderQueue.hpp"

/*
ISSUES:
* Remove Hovered Chunk is a button inside Level Editor.
* Dynamic Entities (like player) coming from ECS are not in sync with layer rendering. DONE
* Tile selection from Tileset selection tool is completely uncomfortable.
* Scene View does not include dynamic entities.
* Tool doesn't allow to delete/create new layers on existing level. DONE
* We don't have camera system. DONE
* We need to be able to stream all chunks in editor mode.
* There's huge offset between mouse & tile hover position in editor. DONE
* Camera needs offset based on player size. DONE
*/

namespace engine::level
{
	class CLevelManager : public engine::core::NonCopyable
	{
	public:

		CLevelManager() = default;
		~CLevelManager();

		bool LoadLevel(const std::string& _name);
		void UnloadLevel();

		bool CreateLevel(SLevelInfo _info);
		bool DeleteLevel(const std::string& _name);
		[[nodiscard]] static std::vector<std::string> ListLevels();
		[[nodiscard]] bool HasLevel() const { return m_Info.has_value(); }

		void Frame(const sf::FloatRect& _focus_world_rect);
		void Render(engine::render::CRenderQueue& _queue) const;
		void RenderDebug(sf::RenderTarget& _target) const;

		[[nodiscard]] sf::Vector2i WorldToTile(sf::Vector2f _world) const;
		[[nodiscard]] sf::Vector2f TileToWorld(sf::Vector2i _tile) const;
		[[nodiscard]] sf::FloatRect TilesToWorld(const sf::IntRect& _tiles) const;
		[[nodiscard]] SChunkCoordinates TileToChunk(sf::Vector2i _tile) const;
		[[nodiscard]] sf::Vector2i TileToLocal(sf::Vector2i _tile) const;
		[[nodiscard]] SChunkCoordinates WorldToChunk(sf::Vector2f _world) const { return TileToChunk(WorldToTile(_world)); }

		bool SetTile(std::string_view _layer, sf::Vector2i _tile, const STile& _value);
		[[nodiscard]] std::optional<STile> GetTile(std::string_view _layer, sf::Vector2i _tile) const;
		void FillTiles(std::string_view _layer, const sf::IntRect& _tiles, const STile& _value);

		bool AddCollisionRect(std::string_view _layer, const sf::IntRect& _tiles, float _friction);
		bool RemoveCollisionRects(std::string_view _layer, const sf::IntRect& _tiles);
		[[nodiscard]] const SCollisionRectData* FindCollisionRect(std::string_view _layer, sf::Vector2i _tile) const;

		bool AddPrefabInstance(std::string_view _layer, sf::Vector2f _world_position, const std::string& _prefab, const std::string& _preset = {});
		bool RemovePrefabInstance(std::string_view _layer, const SChunkCoordinates& _chunk, size_t _index);

		void SaveDirtyChunks();
		void SaveAll();

		[[nodiscard]] CChunk* FindChunk(const SChunkCoordinates& _coord);
		[[nodiscard]] const CChunk* FindChunk(const SChunkCoordinates& _coord) const;
		[[nodiscard]] CChunk& GetOrCreateChunk(const SChunkCoordinates& _coord);
		bool RemoveChunk(const SChunkCoordinates& _coord);
		[[nodiscard]] static std::vector<std::filesystem::path> ListChunkFiles();

		[[nodiscard]] const std::unordered_map<SChunkCoordinates, std::unique_ptr<CChunk>, SChunkCoordinatesHash>& GetChunks() const { return m_Chunks; }
		[[nodiscard]] const SLevelInfo& GetInfo() const { return *m_Info; }
		[[nodiscard]] size_t GetPendingLoadsCount() const { return m_PendingLoads.size(); }

		[[nodiscard]] bool IsLayerVisible(size_t _layer_index) const;
		void SetLayerVisible(size_t _layer_index, bool _visible);

		bool AddLayer(const std::string& _name, ELayerType _type, std::optional<size_t> _insert_at = std::nullopt);
		bool RemoveLayer(size_t _layer_index);
		bool RenameLayer(size_t _layer_index, const std::string& _new_name);
		bool MoveLayer(size_t _from, size_t _to);
		bool SetLayerType(size_t _layer_index, ELayerType _type);

		[[nodiscard]] static std::filesystem::path GetLevelDirectory(const std::string& _level_name);
		[[nodiscard]] static std::filesystem::path GetLevelInfoPath(const std::string& _level_name);
		[[nodiscard]] static std::filesystem::path GetChunkFilePath(const std::string& _level_name, const SChunkCoordinates& _coord);

		static void ResetEntityLayers();

	public:

		bool StreamingEnabled = true;
		static constexpr int DEBUG_COLLIDERS_NONE = -1;
		static constexpr int DEBUG_COLLIDERS_ALL = -2;
		int DebugCollidersLayer = DEBUG_COLLIDERS_NONE;

	private:

		void RequestLoad(const SChunkCoordinates& _coord);
		void OnChunkLoaded(const SChunkCoordinates& _coord, std::optional<SChunkData> _data);
		void Unload(const SChunkCoordinates& _coord);
		void SaveChunk(const CChunk& _chunk) const;
		void FinalizeChunk(CChunk& _chunk);

		bool ApplyLayerChange(const std::function<void(SChunkData&)>& _patch = {});

	private:

		static constexpr sf::Color DEBUG_COLLIDER_COLOR{ 255, 0, 0, 128 };

		std::optional<SLevelInfo> m_Info;
		std::unordered_map<SChunkCoordinates, std::unique_ptr<CChunk>, SChunkCoordinatesHash> m_Chunks;
		std::unordered_set<SChunkCoordinates, SChunkCoordinatesHash> m_PendingLoads;
		std::vector<bool> m_LayerVisibility;
	};
}