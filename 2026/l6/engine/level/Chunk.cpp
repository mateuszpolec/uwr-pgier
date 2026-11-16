#include "Chunk.hpp"

#include <optional>
#include <string>
#include <unordered_map>

#include "engine/core/Logger.hpp"
#include "engine/ecs/Entity.hpp"
#include "engine/ecs/components/Component_Transform.hpp"
#include "engine/game/Game.hpp"
#include "engine/render/RenderCommand.hpp"

namespace engine::level
{
	bool CChunk::BATCH_RENDERING_ENABLED = true;

	CChunk::CChunk(SChunkData _data, sf::Vector2i _tile_size)
		: m_Data(std::move(_data))
		, m_TileSize(_tile_size)
	{
	}

	sf::Vector2f CChunk::GetWorldOrigin() const
	{
		return {
			static_cast<float>(m_Data.Coordinates.X * CHUNK_SIZE * m_TileSize.x),
			static_cast<float>(m_Data.Coordinates.Y * CHUNK_SIZE * m_TileSize.y)
		};
	}

	sf::FloatRect CChunk::GetWorldBounds() const
	{
		return { GetWorldOrigin(), { static_cast<float>(CHUNK_SIZE * m_TileSize.x), static_cast<float>(CHUNK_SIZE * m_TileSize.y) } };
	}

	void CChunk::Spawn()
	{
		Despawn();
		engine::ecs::CPrefabManager& prefab_manager = engine::game::CGame::Get().GetPrefabManager();
		engine::ecs::CEntityManager& entity_manager = engine::game::CGame::Get().GetEntityManager();

		m_Entities.resize(m_Data.Layers.size());

		for (size_t layer_index = 0; layer_index < m_Data.Layers.size(); ++layer_index)
		{
			const SLayerData& layer = m_Data.Layers[layer_index];

			for (const SPrefabInstanceData& instance : layer.Prefabs)
			{
				if (!prefab_manager.HasPrefab(instance.Prefab))
				{
					LOG_WARN("Chunk ({}, {}) layer {} references unknown prefab {}", m_Data.Coordinates.X, m_Data.Coordinates.Y, layer.Name, instance.Prefab);
					continue;
				}

				engine::ecs::CEntity entity = prefab_manager.Instantiate(entity_manager, instance.Prefab, instance.Preset);
				if (!entity.IsValid())
				{
					continue;
				}

				auto* transform = entity.FindComponent<engine::ecs::components::SCoTransform>();
				if (!transform)
				{
					transform = &entity.AddComponent<engine::ecs::components::SCoTransform>();
				}
				transform->Position = GetWorldOrigin() + instance.Position;

				m_Entities[layer_index].push_back(entity.GetId());
			}
		}
	}

	void CChunk::Despawn()
	{
		engine::ecs::CEntityManager& entity_manager = engine::game::CGame::Get().GetEntityManager();

		for (auto& layer_entities : m_Entities)
		{
			for (const engine::ecs::entity_id_t id : layer_entities)
			{
				if (entity_manager.IsValid(id))
				{
					entity_manager.DestroyEntity(id);
				}
			}
		}

		m_Entities.clear();
	}

	void CChunk::AppendQuad(sf::VertexArray& _va, sf::Vector2f _p0, sf::Vector2f _p1, sf::Vector2f _t0, sf::Vector2f _t1, sf::Color _color)
	{
		_va.append({ { _p0.x, _p0.y }, _color, { _t0.x, _t0.y } });
		_va.append({ { _p1.x, _p0.y }, _color, { _t1.x, _t0.y } });
		_va.append({ { _p1.x, _p1.y }, _color, { _t1.x, _t1.y } });
		_va.append({ { _p0.x, _p0.y }, _color, { _t0.x, _t0.y } });
		_va.append({ { _p1.x, _p1.y }, _color, { _t1.x, _t1.y } });
		_va.append({ { _p0.x, _p1.y }, _color, { _t0.x, _t1.y } });
	}

	void CChunk::RebuildGeometry()
	{
		engine::assets::CTilesetManager& tileset_manager = engine::game::CGame::Get().GetTilesetManager();

		const sf::Vector2f origin = GetWorldOrigin();
		const sf::Vector2f tile_size = static_cast<sf::Vector2f>(m_TileSize);

		if (!BATCH_RENDERING_ENABLED)
		{
			m_LayersSprite.clear();
			m_LayersSprite.resize(m_Data.Layers.size());

			for (size_t layer_index = 0; layer_index < m_Data.Layers.size(); ++layer_index)
			{
				const SLayerData& layer = m_Data.Layers[layer_index];
				if (layer.Type != ELayerType::Tiles || !layer.Visible)
				{
					continue;
				}

				std::vector<sf::Sprite>& sprites = m_LayersSprite[layer_index];
				sprites.clear();

				for (int y = 0; y < CHUNK_SIZE; ++y)
				{
					for (int x = 0; x < CHUNK_SIZE; ++x)
					{
						const STile tile = layer.Get(LocalTileIndex(x, y));
						if (!tile.IsValid())
						{
							continue;
						}

						const engine::assets::STileset* tileset = tileset_manager.FindTileset(tile.Tileset);
						if (!tileset || !tileset->IsValid())
						{
							continue;
						}

						const std::optional<sf::IntRect> rect = tileset->GetTileRect(tile.Index);
						if (!rect)
						{
							continue;
						}

						sf::Sprite& sprite = sprites.emplace_back(tileset->Texture.Get());
						sprite.setTextureRect(*rect);
						sprite.setPosition(origin + sf::Vector2f{ x * tile_size.x, y * tile_size.y });
					}
				}
			}

			return;
		}

		m_Layers.clear();
		m_Layers.resize(m_Data.Layers.size());
		for (size_t layer_index = 0; layer_index < m_Data.Layers.size(); ++layer_index)
		{
			const SLayerData& layer = m_Data.Layers[layer_index];
			if (layer.Type != ELayerType::Tiles || !layer.Visible)
			{
				continue;
			}

			std::unordered_map<std::string, size_t> batch_lookup;
			std::vector<STilesetBatch>& batches = m_Layers[layer_index];

			for (int y = 0; y < CHUNK_SIZE; ++y)
			{
				for (int x = 0; x < CHUNK_SIZE; ++x)
				{
					const STile tile = layer.Get(LocalTileIndex(x, y));
					if (!tile.IsValid())
					{
						continue;
					}

					const engine::assets::STileset* tileset = tileset_manager.FindTileset(tile.Tileset);
					if (!tileset || !tileset->IsValid())
					{
						continue;
					}

					const std::optional<sf::IntRect> rect = tileset->GetTileRect(tile.Index);
					if (!rect)
					{
						continue;
					}

					auto it = batch_lookup.find(tile.Tileset);
					if (it == batch_lookup.end())
					{
						batches.push_back(STilesetBatch{ tileset->Texture });
						it = batch_lookup.emplace(tile.Tileset, batches.size() - 1).first;
					}

					const sf::Vector2f p0 = origin + sf::Vector2f{ x * tile_size.x, y * tile_size.y };
					const sf::Vector2f t0 = static_cast<sf::Vector2f>(rect->position);
					const sf::Vector2f t1 = t0 + static_cast<sf::Vector2f>(rect->size);

					AppendQuad(batches[it->second].Vertices, p0, p0 + tile_size, t0, t1, sf::Color::White);
				}
			}
		}

		m_GeometryDirty = false;
	}

	void CChunk::RenderLayer(engine::render::CRenderQueue& _queue, size_t _layer_index) const
	{
		if (_layer_index >= m_Layers.size())
		{
			return;
		}

		if (!BATCH_RENDERING_ENABLED)
		{
			for (const sf::Sprite& sprite : m_LayersSprite[_layer_index])
			{
				_queue.Submit(static_cast<int>(_layer_index), engine::render::SORT_Y_BACKGROUND, sprite);
			}
			return;
		}

		for (const STilesetBatch& batch : m_Layers[_layer_index])
		{
			if (!batch.Texture.IsValid() || batch.Vertices.getVertexCount() == 0)
			{
				continue;
			}

			sf::RenderStates states;
			states.texture = &batch.Texture.Get();
			_queue.Submit(static_cast<int>(_layer_index), engine::render::SORT_Y_BACKGROUND, batch.Vertices, states);
		}
	}

	bool CChunk::IsColliderAt(size_t _layer_index, int _local_x, int _local_y) const
	{
		if (_layer_index >= m_Data.Layers.size())
		{
			return false;
		}
		return m_Data.Layers[_layer_index].IsColliderAt(LocalTileIndex(_local_x, _local_y));
	}

	void CChunk::RenderDebug(sf::RenderTarget& _target, size_t _layer_index, sf::Color _color) const
	{
		if (_layer_index >= m_Data.Layers.size())
		{
			return;
		}

		const SLayerData& layer = m_Data.Layers[_layer_index];
		const sf::Vector2f origin = GetWorldOrigin();
		const sf::Vector2f tile_size = static_cast<sf::Vector2f>(m_TileSize);

		sf::VertexArray va(sf::PrimitiveType::Triangles);

		for (int y = 0; y < CHUNK_SIZE; ++y)
		{
			for (int x = 0; x < CHUNK_SIZE; ++x)
			{
				if (!layer.IsColliderAt(LocalTileIndex(x, y)))
				{
					continue;
				}

				const sf::Vector2f p0 = origin + sf::Vector2f{ x * tile_size.x, y * tile_size.y };
				AppendQuad(va, p0, p0 + tile_size, {}, {}, _color);
			}
		}

		if (va.getVertexCount() > 0)
		{
			_target.draw(va);
		}
	}
}