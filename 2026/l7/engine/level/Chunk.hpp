#pragma once

#include <vector>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include "engine/assets/AssetHandle.hpp"
#include "engine/assets/TilesetManager.hpp"
#include "engine/ecs/EcsTypes.hpp"
#include "engine/ecs/EntityManager.hpp"
#include "engine/ecs/PrefabManager.hpp"
#include "engine/render/RenderQueue.hpp"
#include "ChunkData.hpp"

namespace engine::level
{
	///////////////////////////////////////////////////////////////////////////////////////
	enum class EChunkState
	{
		Ready,
		Unloading
	};

	///////////////////////////////////////////////////////////////////////////////////////
	class CChunk
	{
	public:

		CChunk(SChunkData _data, sf::Vector2i _tile_size);
		~CChunk() = default;

		void Spawn();
		void Despawn();

		void RebuildGeometry();
		void RenderLayer(engine::render::CRenderQueue& _queue, size_t _layer_index) const;
		void RenderDebug(sf::RenderTarget& _target, size_t _layer_index, sf::Color _color) const;

		[[nodiscard]] bool IsColliderAt(size_t _layer_index, int _local_x, int _local_y) const;

		[[nodiscard]] SChunkData& GetData() { return m_Data; }
		[[nodiscard]] const SChunkData& GetData() const { return m_Data; }
		[[nodiscard]] const SChunkCoordinates& GetCoord() const { return m_Data.Coordinates; }
		[[nodiscard]] sf::Vector2f GetWorldOrigin() const;
		[[nodiscard]] sf::FloatRect GetWorldBounds() const;

		[[nodiscard]] const std::vector<std::vector<engine::ecs::entity_id_t>>& GetEntities() const { return m_Entities; }

		[[nodiscard]] bool IsDirty() const { return m_Dirty; }
		[[nodiscard]] bool IsGeometryDirty() const { return m_GeometryDirty; }
		void MarkDirty() { m_Dirty = true; }
		void MarkGeometryDirty() { m_Dirty = true; m_GeometryDirty = true; }
		void ClearDirty() { m_Dirty = false; }

		[[nodiscard]] static constexpr int LocalTileIndex(int _x, int _y) { return _y * CHUNK_SIZE + _x; }

	public:

		EChunkState State = EChunkState::Ready;

	private:

		struct STilesetBatch
		{
			engine::assets::TAssetHandle<sf::Texture> Texture;
			sf::VertexArray Vertices{ sf::PrimitiveType::Triangles };
		};

		static void AppendQuad(sf::VertexArray& _va, sf::Vector2f _p0, sf::Vector2f _p1, sf::Vector2f _t0, sf::Vector2f _t1, sf::Color _color);

	private:

		SChunkData m_Data;
		sf::Vector2i m_TileSize;
		std::vector<std::vector<STilesetBatch>> m_Layers;
		std::vector<std::vector<sf::Sprite>> m_LayersSprite;
		std::vector<std::vector<engine::ecs::entity_id_t>> m_Entities;

		bool m_Dirty = false;
		bool m_GeometryDirty = true;
	};
}