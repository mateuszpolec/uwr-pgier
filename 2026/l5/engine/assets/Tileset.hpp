#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <filesystem>

#include <nlohmann/json.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Texture.hpp>

#include "engine/assets/AssetHandle.hpp"

namespace engine::assets
{
	struct STileset
	{
		static constexpr std::string_view EXTENSION = ".tileset";
		static bool IsTilesetFile(const std::filesystem::path& _path) { return _path.extension() == EXTENSION; }

		std::string TexturePath;
		sf::Vector2i TileSize{ 0, 0 };
		sf::Vector2i Margin{ 0, 0 };
		sf::Vector2i Spacing{ 0, 0 };
		int Columns{ -1 };
		int Rows{ -1 };

		engine::assets::TAssetHandle<sf::Texture> Texture;

		void DetermineColumnsAndRows();
		[[nodiscard]] int GetTileCount() const { return Columns * Rows; }
		[[nodiscard]] std::optional<sf::IntRect> GetTileRect(int _tile_index) const;

		bool IsValid() const { return Texture.IsValid() && TileSize.x > 0 && TileSize.y > 0 && Columns > -1 && Rows > -1; }
	};

	void to_json(nlohmann::json& _j, const STileset& _tileset);
	void from_json(const nlohmann::json& _j, STileset& _tileset);
}