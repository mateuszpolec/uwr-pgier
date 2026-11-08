#if defined(DEBUG)
#pragma once

#include <filesystem>
#include <string>
#include <SFMl/System/Vector2.hpp>

#include "Widget.hpp"

namespace engine::editor
{
	class CAssetsBrowserWidget : public IWidget
	{
	public:
		CAssetsBrowserWidget() = default;
		~CAssetsBrowserWidget() = default;

		void Frame(float) override {};
		void Render() override;

		[[nodiscard]] std::string_view GetName() const override { return "Assets Browser"; }
		[[nodiscard]] std::string_view GetMenuName() const override { return "Windows"; }
		[[nodiscard]] bool GetRenderInMenu() const override { return true; }

	private:

		void RenderDirectory(const std::filesystem::path& _path);
		void RenderContextMenu(const std::filesystem::path& _path);

//////////////// Search ////////////////////////
	private:

		std::string m_SearchFilter;
		bool MatchesFilter(std::string_view _name) const;
		bool DirectoryContentMatchesFilter(const std::filesystem::path& _path) const;

//////////////// Tileset generation ////////////////////////
	private:

		void RenderGenerateTilesetPopup();

		bool m_OpenGenerateTilesetPopup = false;
		std::string m_PendingTilesetTexturePath;

		sf::Vector2i m_TilesetTileSize{ 0, 0 };
		sf::Vector2i m_TilesetMargin{ 0, 0 };
		sf::Vector2i m_TilesetSpacing{ 0, 0 };
	};
}

#endif
