#pragma once

#include <filesystem>
#include <format>
#include <string>
#include <string_view>
#include <vector>

#include <nlohmann/json.hpp>
#include <SFML/System/Vector2.hpp>

#include "engine/filesystem/JsonSerializationSfml.hpp"
#include "engine/filesystem/Paths.hpp"
#include "engine/core/Hash.hpp"
#include "LevelTypes.hpp"
#include "ChunkData.hpp"

namespace engine::level
{
	///////////////////////////////////////////////////////////////////////////////////////
	struct SLayerInfo
	{
		std::string Name;
		ELayerType Type = ELayerType::Tiles;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SLayerInfo, Name, Type)

///////////////////////////////////////////////////////////////////////////////////////
	struct SLevelInfo
	{
		static constexpr std::string_view EXTENSION = ".levelinfo";

		std::string Name;
		sf::Vector2i TileSize{ 32, 32 };
		std::vector<SLayerInfo> Layers;

		[[nodiscard]] int FindLayerIndex(std::string_view _name) const
		{
			for (size_t i = 0; i < Layers.size(); ++i)
			{
				if (Layers[i].Name == _name)
				{
					return static_cast<int>(i);
				}
			}
			return -1;
		}

		[[nodiscard]] SChunkData CreateEmptyChunk(const SChunkCoordinates& _coord) const
		{
			SChunkData data;
			data.Coordinates = _coord;
			SyncChunkLayers(data);
			return data;
		}

		void SyncChunkLayers(SChunkData& _data) const
		{
			std::vector<SLayerData> layers;
			layers.reserve(Layers.size());

			for (const SLayerInfo& info : Layers)
			{
				if (SLayerData* existing = _data.FindLayer(info.Name))
				{
					existing->Type = info.Type;
					existing->SetDefaultSize();
					layers.push_back(std::move(*existing));
					continue;
				}

				SLayerData layer;
				layer.Name = info.Name;
				layer.Type = info.Type;
				layers.push_back(std::move(layer));
			}

			_data.Layers = std::move(layers);
		}
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SLevelInfo, Name, TileSize, Layers)
}