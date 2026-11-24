#include <cmath>
#include <format>
#include <vector>

#include "LevelManager.hpp"
#include "engine/core/Hash.hpp"
#include "engine/core/Logger.hpp"
#include "engine/filesystem/JsonSerialization.hpp"
#include "engine/filesystem/Paths.hpp"
#include "engine/game/Game.hpp"
#include "engine/ecs/components/Component_Renderable.hpp"

namespace engine::level
{
	CLevelManager::~CLevelManager()
	{
		UnloadLevel();
	}

	bool CLevelManager::LoadLevel(const std::string& _name)
	{
		UnloadLevel();

		const std::filesystem::path path = GetLevelInfoPath(_name);
		std::optional<SLevelInfo> info = engine::filesystem::Load<SLevelInfo>(path);
		if (!info)
		{
			LOG_ERROR("Level info not found: {}", path.string());
			return false;
		}

		if (info->Layers.empty() || info->TileSize.x <= 0 || info->TileSize.y <= 0)
		{
			LOG_ERROR("Level '{}' is invalid (no layers or bad tile size)", _name);
			return false;
		}

		m_Info = std::move(*info);
		LOG_INFO("Level '{}' loaded, {} layers, streaming radius {}", m_Info->Name, m_Info->Layers.size(), engine::game::CGame::Get().GetGameSettings().GetStreamingRadius());
		return true;
	}

	bool CLevelManager::CreateLevel(SLevelInfo _info)
	{
		if (_info.Name.empty() || _info.Layers.empty())
		{
			LOG_ERROR("Cannot create level: name or layers missing");
			return false;
		}

		if (!engine::filesystem::Save(GetLevelInfoPath(_info.Name), _info))
		{
			LOG_ERROR("Failed to save level info '{}'", _info.Name);
			return false;
		}

		UnloadLevel();
		m_Info = std::move(_info);
		LOG_INFO("Level '{}' created", m_Info->Name);
		return true;
	}

	bool CLevelManager::DeleteLevel(const std::string& _name)
	{
		if (m_Info && m_Info->Name == _name)
		{
			for (auto& [coord, chunk] : m_Chunks)
			{
				chunk->Despawn();
			}
			m_Chunks.clear();
			m_PendingLoads.clear();
			m_Info.reset();
			m_LayerVisibility.clear();
		}


		std::error_code ec;
		const auto removed = std::filesystem::remove_all(GetLevelDirectory(_name), ec);
		if (ec)
		{
			LOG_ERROR("Failed to delete level {}: {}", _name, ec.message());
			return false;
		}

		LOG_INFO("Deleted level {} ({} files)", _name, removed);
		return true;
	}

	std::vector<std::string> CLevelManager::ListLevels()
	{
		std::vector<std::string> levels;
		const std::filesystem::path root = engine::filesystem::Assets() / "levels";

		std::error_code ec;
		if (!std::filesystem::is_directory(root, ec))
		{
			return levels;
		}

		for (const auto& entry : std::filesystem::directory_iterator(root, ec))
		{
			if (!entry.is_directory())
			{
				continue;
			}

			const std::string name = entry.path().filename().string();
			if (std::filesystem::exists(GetLevelInfoPath(name), ec))
			{
				levels.push_back(name);
			}
		}

		std::sort(levels.begin(), levels.end());
		return levels;
	}

	void CLevelManager::UnloadLevel()
	{
		if (!m_Info)
		{
			return;
		}

		SaveDirtyChunks();

		for (auto& [coord, chunk] : m_Chunks)
		{
			chunk->Despawn();
		}

		m_Chunks.clear();
		m_PendingLoads.clear();
		m_Info.reset();
	}

	void CLevelManager::Frame(const sf::FloatRect& _focus_world_rect)
	{
		if (!m_Info)
		{
			return;
		}

		if (StreamingEnabled)
		{
			const SChunkCoordinates min_chunk = WorldToChunk(_focus_world_rect.position);
			const SChunkCoordinates max_chunk = WorldToChunk(_focus_world_rect.position + _focus_world_rect.size);

			const int load_radius = std::max(0, engine::game::CGame::Get().GetGameSettings().GetStreamingRadius());
			const int keep_radius = load_radius + 1; // +1 as a margin error buffer to avoid loading & unloading chunks on the edge of the radius

			auto fn_in_range = [&min_chunk, &max_chunk](const SChunkCoordinates& _coord, int _radius)
				{
					return _coord.X >= min_chunk.X - _radius && _coord.X <= max_chunk.X + _radius
						&& _coord.Y >= min_chunk.Y - _radius && _coord.Y <= max_chunk.Y + _radius;
				};

			for (int y = min_chunk.Y - load_radius; y <= max_chunk.Y + load_radius; ++y)
			{
				for (int x = min_chunk.X - load_radius; x <= max_chunk.X + load_radius; ++x)
				{
					const SChunkCoordinates coord{ x, y };
					if (!m_Chunks.contains(coord) && !m_PendingLoads.contains(coord))
					{
						RequestLoad(coord);
					}
				}
			}

			std::vector<SChunkCoordinates> to_unload;
			for (const auto& [coord, chunk] : m_Chunks)
			{
				if (chunk->State == EChunkState::Ready && !fn_in_range(coord, keep_radius))
				{
					to_unload.push_back(coord);
				}
			}
			for (const SChunkCoordinates& coord : to_unload)
			{
				Unload(coord);
			}

			std::erase_if(m_PendingLoads, [&](const SChunkCoordinates& _coord) { return !fn_in_range(_coord, keep_radius); });
		}

		for (auto& [coord, chunk] : m_Chunks)
		{
			if (chunk->IsGeometryDirty())
			{
				chunk->RebuildGeometry();
			}
		}
	}

	void CLevelManager::Render(engine::render::CRenderQueue& _queue) const
	{
		if (!m_Info)
		{
			return;
		}

		for (size_t layer_index = 0; layer_index < m_Info->Layers.size(); ++layer_index)
		{
			if (!IsLayerVisible(layer_index))
			{
				continue;
			}

			for (const auto& [coord, chunk] : m_Chunks)
			{
				chunk->RenderLayer(_queue, layer_index);
			}
		}
	}

	void CLevelManager::RenderDebug(sf::RenderTarget& _target) const
	{
		if (!m_Info || DebugCollidersLayer == DEBUG_COLLIDERS_NONE)
		{
			return;
		}

		for (size_t layer_index = 0; layer_index < m_Info->Layers.size(); ++layer_index)
		{
			const bool selected = DebugCollidersLayer == DEBUG_COLLIDERS_ALL || DebugCollidersLayer == static_cast<int>(layer_index);
			if (!selected || !IsLayerVisible(layer_index))
			{
				continue;
			}

			for (const auto& [coord, chunk] : m_Chunks)
			{
				chunk->RenderDebug(_target, layer_index, DEBUG_COLLIDER_COLOR);
			}
		}
	}

	sf::Vector2i CLevelManager::WorldToTile(sf::Vector2f _world) const
	{
		return {
			static_cast<int>(std::floor(_world.x / static_cast<float>(m_Info->TileSize.x))),
			static_cast<int>(std::floor(_world.y / static_cast<float>(m_Info->TileSize.y)))
		};
	}

	sf::Vector2f CLevelManager::TileToWorld(sf::Vector2i _tile) const
	{
		return { static_cast<float>(_tile.x * m_Info->TileSize.x), static_cast<float>(_tile.y * m_Info->TileSize.y) };
	}

	sf::FloatRect CLevelManager::TilesToWorld(const sf::IntRect& _tiles) const
	{
		return { TileToWorld(_tiles.position), TileToWorld(_tiles.size) };
	}

	SChunkCoordinates CLevelManager::TileToChunk(sf::Vector2i _tile) const
	{
		auto fn_floor_div = [](int _a, int _b) -> int
		{
			return static_cast<int>(std::floor(static_cast<float>(_a) / static_cast<float>(_b)));
		};

		return { fn_floor_div(_tile.x, CHUNK_SIZE), fn_floor_div(_tile.y, CHUNK_SIZE) };
	}

	sf::Vector2i CLevelManager::TileToLocal(sf::Vector2i _tile) const
	{
		const SChunkCoordinates chunk = TileToChunk(_tile);
		return { _tile.x - chunk.X * CHUNK_SIZE, _tile.y - chunk.Y * CHUNK_SIZE };
	}

	bool CLevelManager::SetTile(std::string_view _layer, sf::Vector2i _tile, const STile& _value)
	{
		if (!m_Info)
		{
			return false;
		}

		CChunk& chunk = GetOrCreateChunk(TileToChunk(_tile));
		SLayerData* layer = chunk.GetData().FindLayer(_layer);
		if (!layer)
		{
			LOG_WARN("Layer '{}' does not exist in level '{}'", _layer, m_Info->Name);
			return false;
		}

		const sf::Vector2i local = TileToLocal(_tile);
		const int index = CChunk::LocalTileIndex(local.x, local.y);
		if (layer->Get(index) == _value)
		{
			return false;
		}

		layer->Set(index, _value);
		chunk.MarkGeometryDirty();
		return true;
	}

	std::optional<STile> CLevelManager::GetTile(std::string_view _layer, sf::Vector2i _tile) const
	{
		if (!m_Info)
		{
			return std::nullopt;
		}

		const CChunk* chunk = FindChunk(TileToChunk(_tile));
		if (!chunk)
		{
			return std::nullopt;
		}

		const SLayerData* layer = chunk->GetData().FindLayer(_layer);
		if (!layer)
		{
			return std::nullopt;
		}

		const sf::Vector2i local = TileToLocal(_tile);
		return layer->Get(CChunk::LocalTileIndex(local.x, local.y));
	}

	void CLevelManager::FillTiles(std::string_view _layer, const sf::IntRect& _tiles, const STile& _value)
	{
		for (int y = _tiles.position.y; y < _tiles.position.y + _tiles.size.y; ++y)
		{
			for (int x = _tiles.position.x; x < _tiles.position.x + _tiles.size.x; ++x)
			{
				SetTile(_layer, { x, y }, _value);
			}
		}
	}

	bool CLevelManager::AddCollisionRect(std::string_view _layer, const sf::IntRect& _tiles, float _friction)
	{
		if (!m_Info)
		{
			return false;
		}

		// The rect is owned by the chunk containing its top-left tile
		CChunk& chunk = GetOrCreateChunk(TileToChunk(_tiles.position));
		SLayerData* layer = chunk.GetData().FindLayer(_layer);
		if (!layer)
		{
			LOG_WARN("Layer '{}' does not exist in level '{}'", _layer, m_Info->Name);
			return false;
		}

		const sf::FloatRect world = TilesToWorld(_tiles);
		layer->CollisionRects.push_back({ world.position - chunk.GetWorldOrigin(), world.size, _friction });
		chunk.MarkDirty();
		return true;
	}

	// Removes every rect of the layer that overlaps the given tiles
	bool CLevelManager::RemoveCollisionRects(std::string_view _layer, const sf::IntRect& _tiles)
	{
		if (!m_Info)
		{
			return false;
		}

		const sf::FloatRect world = TilesToWorld(_tiles);
		bool removed = false;

		for (auto& [coord, chunk] : m_Chunks)
		{
			SLayerData* layer = chunk->GetData().FindLayer(_layer);
			if (!layer)
			{
				continue;
			}

			const sf::Vector2f origin = chunk->GetWorldOrigin();
			const size_t count = std::erase_if(layer->CollisionRects, [&world, &origin](const SCollisionRectData& _rect)
				{
					return world.findIntersection(_rect.GetWorldBounds(origin)).has_value();
				});

			if (count > 0)
			{
				chunk->MarkDirty();
				removed = true;
			}
		}

		return removed;
	}

	const SCollisionRectData* CLevelManager::FindCollisionRect(std::string_view _layer, sf::Vector2i _tile) const
	{
		if (!m_Info)
		{
			return nullptr;
		}

		const sf::FloatRect world = TilesToWorld({ _tile, { 1, 1 } });

		for (const auto& [coord, chunk] : m_Chunks)
		{
			const SLayerData* layer = chunk->GetData().FindLayer(_layer);
			if (!layer)
			{
				continue;
			}

			for (const SCollisionRectData& rect : layer->CollisionRects)
			{
				if (world.findIntersection(rect.GetWorldBounds(chunk->GetWorldOrigin())))
				{
					return &rect;
				}
			}
		}

		return nullptr;
	}

	bool CLevelManager::AddPrefabInstance(std::string_view _layer, sf::Vector2f _world_position, const std::string& _prefab, const std::string& _preset)
	{
		if (!m_Info)
		{
			return false;
		}

		if (!engine::game::CGame::Get().GetPrefabManager().HasPrefab(_prefab))
		{
			LOG_WARN("Unknown prefab '{}'", _prefab);
			return false;
		}

		CChunk& chunk = GetOrCreateChunk(WorldToChunk(_world_position));
		SLayerData* layer = chunk.GetData().FindLayer(_layer);
		if (!layer)
		{
			LOG_WARN("Layer '{}' does not exist in level '{}'", _layer, m_Info->Name);
			return false;
		}

		layer->Prefabs.push_back({ _prefab, _preset, _world_position - chunk.GetWorldOrigin() });
		chunk.MarkDirty();
		chunk.Spawn();
		return true;
	}


	bool CLevelManager::RemovePrefabInstance(std::string_view _layer, const SChunkCoordinates& _chunk, size_t _index)
	{
		CChunk* chunk = FindChunk(_chunk);
		if (!chunk)
		{
			return false;
		}

		SLayerData* layer = chunk->GetData().FindLayer(_layer);
		if (!layer || _index >= layer->Prefabs.size())
		{
			return false;
		}

		layer->Prefabs.erase(layer->Prefabs.begin() + static_cast<std::ptrdiff_t>(_index));
		chunk->MarkDirty();
		chunk->Spawn();
		return true;
	}

	void CLevelManager::SaveDirtyChunks()
	{
		for (auto& [coord, chunk] : m_Chunks)
		{
			if (chunk->IsDirty())
			{
				SaveChunk(*chunk);
				chunk->ClearDirty();
			}
		}
	}

	void CLevelManager::SaveAll()
	{
		for (auto& [coord, chunk] : m_Chunks)
		{
			SaveChunk(*chunk);
			chunk->ClearDirty();
		}
	}

	void CLevelManager::SaveChunk(const CChunk& _chunk) const
	{
		if (!engine::filesystem::Save(GetChunkFilePath(m_Info->Name, _chunk.GetCoord()), _chunk.GetData(), 0))
		{
			LOG_ERROR("Failed to save chunk ({}, {})", _chunk.GetCoord().X, _chunk.GetCoord().Y);
		}
	}

	CChunk* CLevelManager::FindChunk(const SChunkCoordinates& _coord)
	{
		const auto it = m_Chunks.find(_coord);
		return it != m_Chunks.end() ? it->second.get() : nullptr;
	}

	const CChunk* CLevelManager::FindChunk(const SChunkCoordinates& _coord) const
	{
		const auto it = m_Chunks.find(_coord);
		return it != m_Chunks.end() ? it->second.get() : nullptr;
	}

	CChunk& CLevelManager::GetOrCreateChunk(const SChunkCoordinates& _coord)
	{
		if (CChunk* chunk = FindChunk(_coord))
		{
			return *chunk;
		}

		m_PendingLoads.erase(_coord);

		std::optional<SChunkData> loaded = engine::filesystem::Load<SChunkData>(GetChunkFilePath(m_Info->Name, _coord));
		SChunkData data = loaded ? std::move(*loaded) : m_Info->CreateEmptyChunk(_coord);
		data.Coordinates = _coord;
		m_Info->SyncChunkLayers(data);

		auto chunk = std::make_unique<CChunk>(std::move(data), m_Info->TileSize);
		CChunk& ref = *chunk;
		m_Chunks.emplace(_coord, std::move(chunk));
		FinalizeChunk(ref);
		return ref;
	}

	bool CLevelManager::RemoveChunk(const SChunkCoordinates& _coord)
	{
		if (!m_Info)
		{
			return false;
		}

		if (const auto it = m_Chunks.find(_coord); it != m_Chunks.end())
		{
			it->second->Despawn();
			m_Chunks.erase(it);
		}
		m_PendingLoads.erase(_coord);

		std::error_code ec;
		std::filesystem::remove(GetChunkFilePath(m_Info->Name, _coord), ec);
		if (ec)
		{
			LOG_WARN("Failed to remove chunk file ({}, {}): {}", _coord.X, _coord.Y, ec.message());
			return false;
		}

		LOG_INFO("Removed chunk ({}, {})", _coord.X, _coord.Y);
		return true;
	}

	std::vector<std::filesystem::path> CLevelManager::ListChunkFiles()
	{
		const SLevelInfo& level_info = engine::game::CGame::Get().GetLevelManager().GetInfo();

		std::vector<std::filesystem::path> files;
		std::error_code ec;
		for (const auto& entry : std::filesystem::directory_iterator(GetLevelDirectory(level_info.Name), ec))
		{
			if (entry.is_regular_file() && entry.path().extension() == SChunkData::EXTENSION)
			{
				files.push_back(entry.path());
			}
		}
		
		return files;
	}

	void CLevelManager::RequestLoad(const SChunkCoordinates& _coord)
	{
		m_PendingLoads.insert(_coord);

		const std::filesystem::path path = GetChunkFilePath(m_Info->Name, _coord);

		engine::game::CGame::Get().GetJobSystem().Submit<std::optional<SChunkData>>([path]() -> std::optional<SChunkData>
			{
				return engine::filesystem::Load<SChunkData>(path);
			},
			[this, _coord](std::optional<SChunkData> _data)
			{
				OnChunkLoaded(_coord, std::move(_data));
			});
	}

	void CLevelManager::OnChunkLoaded(const SChunkCoordinates& _coord, std::optional<SChunkData> _data)
	{
		if (!m_Info || !m_PendingLoads.erase(_coord) || m_Chunks.contains(_coord))
		{
			return;
		}

		SChunkData data = _data ? std::move(*_data) : m_Info->CreateEmptyChunk(_coord);
		data.Coordinates = _coord;

		auto chunk = std::make_unique<CChunk>(std::move(data), m_Info->TileSize);
		CChunk& ref = *chunk;
		m_Chunks.emplace(_coord, std::move(chunk));
		FinalizeChunk(ref);
	}

	void CLevelManager::FinalizeChunk(CChunk& _chunk)
	{
		_chunk.State = EChunkState::Ready;
		_chunk.RebuildGeometry();
		_chunk.Spawn();
	}

	void CLevelManager::Unload(const SChunkCoordinates& _coord)
	{
		const auto it = m_Chunks.find(_coord);
		if (it == m_Chunks.end())
		{
			return;
		}

		CChunk& chunk = *it->second;
		chunk.State = EChunkState::Unloading;
		chunk.Despawn();

		if (chunk.IsDirty())
		{
			const std::filesystem::path path = GetChunkFilePath(m_Info->Name, _coord);
			engine::game::CGame::Get().GetJobSystem().Submit(
				[path, data = chunk.GetData()]()
				{
					engine::filesystem::Save(path, data, 0);
				},
				{});
		}

		m_Chunks.erase(it);
	}

	bool CLevelManager::IsLayerVisible(size_t _layer_index) const
	{
		return _layer_index < m_LayerVisibility.size() ? m_LayerVisibility[_layer_index] : true;
	}

	void CLevelManager::SetLayerVisible(size_t _layer_index, bool _visible)
	{
		if (!m_Info || _layer_index >= m_Info->Layers.size())
		{
			return;
		}

		m_LayerVisibility.resize(m_Info->Layers.size(), true);
		m_LayerVisibility[_layer_index] = _visible;
	}

	bool CLevelManager::ApplyLayerChange(const std::function<void(SChunkData&)>& _patch)
	{
		if (!m_Info)
		{
			return false;
		}

		// 1. Persist the new layer list.
		if (!engine::filesystem::Save(GetLevelInfoPath(m_Info->Name), *m_Info))
		{
			LOG_ERROR("Failed to save level info '{}'", m_Info->Name);
			return false;
		}

		// 2. Loaded chunks: patch, sync, rebuild, respawn.
		for (auto& [coord, chunk] : m_Chunks)
		{
			SChunkData& data = chunk->GetData();
			if (_patch)
			{
				_patch(data);
			}
			m_Info->SyncChunkLayers(data);
			chunk->MarkGeometryDirty();
			chunk->RebuildGeometry();
			chunk->Spawn();
			SaveChunk(*chunk);
			chunk->ClearDirty();
		}

		// 3. Chunks on disk that are not loaded: patch + sync + rewrite.
		for (const std::filesystem::path& path : ListChunkFiles())
		{
			std::optional<SChunkData> data = engine::filesystem::Load<SChunkData>(path);
			if (!data || m_Chunks.contains(data->Coordinates))
			{
				continue;
			}

			if (_patch)
			{
				_patch(*data);
			}
			m_Info->SyncChunkLayers(*data);

			if (!engine::filesystem::Save(path, *data, 0))
			{
				LOG_ERROR("Failed to rewrite chunk file {}", path.string());
			}
		}

		// 4. Indices shifted - reset per-layer runtime state.
		m_LayerVisibility.assign(m_Info->Layers.size(), true);
		if (DebugCollidersLayer >= static_cast<int>(m_Info->Layers.size()))
		{
			DebugCollidersLayer = DEBUG_COLLIDERS_NONE;
		}
		ResetEntityLayers();

		return true;
	}

	bool CLevelManager::AddLayer(const std::string& _name, ELayerType _type, std::optional<size_t> _insert_at)
	{
		if (!m_Info || _name.empty())
		{
			return false;
		}

		if (m_Info->FindLayerIndex(_name) >= 0)
		{
			LOG_WARN("Layer '{}' already exists", _name);
			return false;
		}

		const size_t at = std::min(_insert_at.value_or(m_Info->Layers.size()), m_Info->Layers.size());
		m_Info->Layers.insert(m_Info->Layers.begin() + static_cast<std::ptrdiff_t>(at), SLayerInfo{ _name, _type });

		LOG_INFO("Added layer '{}' at {}", _name, at);
		return ApplyLayerChange();
	}

	bool CLevelManager::RemoveLayer(size_t _layer_index)
	{
		if (!m_Info || _layer_index >= m_Info->Layers.size() || m_Info->Layers.size() <= 1)
		{
			return false;
		}

		const std::string name = m_Info->Layers[_layer_index].Name;
		m_Info->Layers.erase(m_Info->Layers.begin() + static_cast<std::ptrdiff_t>(_layer_index));

		LOG_INFO("Removed layer '{}'", name);
		return ApplyLayerChange();
	}

	bool CLevelManager::RenameLayer(size_t _layer_index, const std::string& _new_name)
	{
		if (!m_Info || _layer_index >= m_Info->Layers.size() || _new_name.empty())
		{
			return false;
		}

		if (m_Info->FindLayerIndex(_new_name) >= 0)
		{
			LOG_WARN("Layer '{}' already exists", _new_name);
			return false;
		}

		const std::string old_name = m_Info->Layers[_layer_index].Name;
		m_Info->Layers[_layer_index].Name = _new_name;

		LOG_INFO("Renamed layer '{}' -> '{}'", old_name, _new_name);
		return ApplyLayerChange([&old_name, &_new_name](SChunkData& _data)
			{
				if (SLayerData* layer = _data.FindLayer(old_name))
				{
					layer->Name = _new_name;
				}
			});
	}

	bool CLevelManager::MoveLayer(size_t _from, size_t _to)
	{
		if (!m_Info || _from >= m_Info->Layers.size() || _to >= m_Info->Layers.size() || _from == _to)
		{
			return false;
		}

		SLayerInfo moved = std::move(m_Info->Layers[_from]);
		m_Info->Layers.erase(m_Info->Layers.begin() + static_cast<std::ptrdiff_t>(_from));
		m_Info->Layers.insert(m_Info->Layers.begin() + static_cast<std::ptrdiff_t>(_to), std::move(moved));

		return ApplyLayerChange();
	}

	bool CLevelManager::SetLayerType(size_t _layer_index, ELayerType _type)
	{
		if (!m_Info || _layer_index >= m_Info->Layers.size() || m_Info->Layers[_layer_index].Type == _type)
		{
			return false;
		}

		m_Info->Layers[_layer_index].Type = _type;
		return ApplyLayerChange();
	}

	std::filesystem::path CLevelManager::GetLevelDirectory(const std::string& _level_name)
	{
		return engine::filesystem::Assets() / "levels" / _level_name;
	}

	std::filesystem::path CLevelManager::GetLevelInfoPath(const std::string& _level_name)
	{
		std::filesystem::path path = GetLevelDirectory(_level_name) / _level_name;
		path += SLevelInfo::EXTENSION;
		return path;
	}

	std::filesystem::path CLevelManager::GetChunkFilePath(const std::string& _level_name, const SChunkCoordinates& _coord)
	{
		constexpr std::string_view SALT = "uwr-pgier::chunk::v1";
		const std::string key = std::format("{}:{}:{}", SALT, _coord.X, _coord.Y);

		std::filesystem::path path = GetLevelDirectory(_level_name) / std::format("{:016x}", engine::core::Fnv1a64(key));
		path += SChunkData::EXTENSION;
		return path;
	}

	void CLevelManager::ResetEntityLayers()
	{
		engine::game::CGame::Get().GetEntityManager().ForEach<engine::ecs::components::SCoRenderable>(
			[](engine::ecs::entity_id_t, engine::ecs::components::SCoRenderable& _renderable)
			{
				if (!_renderable.Layer.empty())
				{
					_renderable.LayerIndex = -1;
				}
			});
	}
}