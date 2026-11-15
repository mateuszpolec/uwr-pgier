#pragma once

#include <filesystem>
#include <unordered_map>
#include <string>

#include "Tileset.hpp"

namespace engine::assets
{
	class CTilesetManager
	{
		public:

			CTilesetManager() = default;
			~CTilesetManager() = default;

			bool SaveTileset(const std::string& _name, const STileset& _tileset);
			bool RemoveTileset(const std::string& _name);

			void LoadDirectory(const std::filesystem::path& _path);
			void LoadTileset(const std::filesystem::path& _path);

			[[nodiscard]] bool HasTileset(const std::string& _name) const;
			[[nodiscard]] const STileset* FindTileset(const std::string& _name) const;

			size_t GetTilesetCount() const { return m_Tilesets.size(); }
			const std::unordered_map<std::string, STileset>& GetTilesets() const { return m_Tilesets; }

			[[nodiscard]] static std::filesystem::path GetTilesetFilePath(const std::string& _name);

		private:

			std::unordered_map<std::string, STileset> m_Tilesets;
	};
}