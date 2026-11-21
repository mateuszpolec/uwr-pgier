#include "Tileset.hpp"
#include "engine/core/Logger.hpp"
#include "AssetManager.hpp"
#include "engine/game/Game.hpp"
#include "engine/filesystem/JsonSerializationSfml.hpp"

namespace engine::assets
{
	void STileset::DetermineColumnsAndRows()
	{
		const sf::Vector2u texture_size = Texture.Get().getSize();

		const int step_x = TileSize.x + Spacing.x;
		const int step_y = TileSize.y + Spacing.y;

		Columns = (static_cast<int>(texture_size.x) - 2 * Margin.x + Spacing.x) / step_x;
		Rows = (static_cast<int>(texture_size.y) - 2 * Margin.y + Spacing.y) / step_y;
	}

	std::optional<sf::IntRect> STileset::GetTileRect(tile_index_t _tile_index) const
	{
		if (!IsValid() || _tile_index < 0 || _tile_index >= GetTileCount())
		{
			LOG_WARN("Invalid tile index {} for tileset '{}'", _tile_index, TexturePath);
			return std::optional<sf::IntRect>();
		}

		const int x = _tile_index % Columns;
		const int y = _tile_index / Columns;
		const int left = Margin.x + x * (TileSize.x + Spacing.x);
		const int top = Margin.y + y * (TileSize.y + Spacing.y);
		return sf::IntRect({ left, top }, { TileSize.x, TileSize.y});
	}

	void to_json(nlohmann::json& _j, const STileset& _tileset)
	{
		_j = nlohmann::json{
			{ "TexturePath", _tileset.TexturePath },
			{ "TileSize", _tileset.TileSize },
			{ "Margin", _tileset.Margin },
			{ "Spacing", _tileset.Spacing },
			{ "Columns", _tileset.Columns },
			{ "Rows", _tileset.Rows }
		};
	}

	void from_json(const nlohmann::json& _j, STileset& _tileset)
	{
		_j.at("TexturePath").get_to(_tileset.TexturePath);
		_j.at("TileSize").get_to(_tileset.TileSize);
		_j.at("Margin").get_to(_tileset.Margin);
		_j.at("Spacing").get_to(_tileset.Spacing);
		_j.at("Columns").get_to(_tileset.Columns);
		_j.at("Rows").get_to(_tileset.Rows);
		if (!_tileset.TexturePath.empty())
		{
			auto& asset_manager = engine::game::CGame::Get().GetAssetsManager();
			auto texture_handle = asset_manager.Get<sf::Texture>(_tileset.TexturePath);
			if (!texture_handle.IsValid())
			{
				LOG_WARN("Failed to load texture '{}' for tileset", _tileset.TexturePath);
			}
			else
			{
				_tileset.Texture = std::move(texture_handle);
			}
		}
	}
}