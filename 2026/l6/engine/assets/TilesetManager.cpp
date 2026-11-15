#include "TilesetManager.hpp"

#include "engine/core/Logger.hpp"
#include "engine/filesystem/JsonSerialization.hpp"

namespace engine::assets
{
	void CTilesetManager::LoadDirectory(const std::filesystem::path& _path)
	{
		if (!std::filesystem::exists(_path) || !std::filesystem::is_directory(_path))
		{
			LOG_ERROR("Tileset directory does not exist: {}", _path.string());
			return;
		}

		for (const auto& entry : std::filesystem::directory_iterator(_path))
		{
			if (STileset::IsTilesetFile(_path))
			{
				continue;
			}

			LoadTileset(entry.path());
		}
	}

	void CTilesetManager::LoadTileset(const std::filesystem::path& _path)
	{
		std::optional<STileset> tileset = engine::filesystem::Load<STileset>(_path);
		if (!tileset)
		{
			LOG_ERROR("Failed to load tileset: {}", _path.string());
			return;
		}

		const std::string name = _path.stem().string();
		m_Tilesets[name] = std::move(*tileset);
		LOG_INFO("Loaded tileset '{}'", name);
	}

	bool CTilesetManager::SaveTileset(const std::string& _name, const STileset& _tileset)
	{
		if (_name.empty() || !_tileset.IsValid())
		{
			LOG_ERROR("Cannot save tileset '{}': invalid name or tileset", _name);
			return false;
		}

		if (!engine::filesystem::Save(GetTilesetFilePath(_name), _tileset))
		{
			LOG_ERROR("Failed to save tileset '{}'", _name);
			return false;
		}

		m_Tilesets.insert_or_assign(_name, _tileset);
		LOG_INFO("Saved tileset '{}'", _name);
		return true;
	}

	bool CTilesetManager::RemoveTileset(const std::string& _name)
	{
		std::error_code ec;
		std::filesystem::remove(GetTilesetFilePath(_name), ec);
		if (ec)
		{
			LOG_WARN("Failed to remove tileset file '{}': {}", _name, ec.message());
		}

		const bool removed = m_Tilesets.erase(_name) > 0;
		if (removed)
		{
			LOG_INFO("Removed tileset '{}'", _name);
		}
		return removed;
	}

	[[nodiscard]] bool CTilesetManager::HasTileset(const std::string& _name) const
	{
		return m_Tilesets.find(_name) != m_Tilesets.end();
	}

	[[nodiscard]] const STileset* CTilesetManager::FindTileset(const std::string& _name) const
	{
		auto it = m_Tilesets.find(_name);
		if (it == m_Tilesets.end())
		{
			LOG_ERROR("Tileset not found: {}", _name);
			return nullptr;
		}

		return &it->second;
	}

	std::filesystem::path CTilesetManager::GetTilesetFilePath(const std::string& _name)
	{
		std::filesystem::path path = engine::filesystem::Assets() / "tilesets" / _name;
		path += STileset::EXTENSION;
		return path;
	}
}