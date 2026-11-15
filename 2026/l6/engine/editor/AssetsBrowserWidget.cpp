#if defined(DEBUG)
#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include "AssetsBrowserWidget.hpp"
#include "engine/filesystem/Paths.hpp"
#include "engine/assets/AssetTypes.hpp"
#include "engine/assets/AssetManager.hpp"
#include "engine/assets/Tileset.hpp"
#include "engine/game/Game.hpp"
#include "engine/filesystem/JsonSerialization.hpp"
#include "notifications/NotificationType.hpp"

namespace engine::editor
{
	void CAssetsBrowserWidget::Render()
	{
		if (ImGui::Begin("Assets Browser", &m_Visible))
		{
			ImGui::InputTextWithHint("##Search", "Search files and folders...", &m_SearchFilter);

			RenderDirectory(engine::filesystem::Assets());
			RenderGenerateTilesetPopup();
		}

		ImGui::End();
	}

	void CAssetsBrowserWidget::RenderDirectory(const std::filesystem::path& _path)
	{
		const bool filtering = !m_SearchFilter.empty();

		for (const auto& entry : std::filesystem::directory_iterator(_path))
		{
			const std::string label = entry.path().filename().string();

			if (entry.is_directory())
			{
				if (filtering && !MatchesFilter(label) && !DirectoryContentMatchesFilter(entry.path()))
				{
					continue;
				}

				if (filtering)
				{
					ImGui::SetNextItemOpen(true, ImGuiCond_Always);
				}

				if (ImGui::TreeNode(label.c_str()))
				{
					RenderDirectory(entry.path());
					ImGui::TreePop();
				}

				continue;
			}

			if (filtering && !MatchesFilter(label))
			{
				continue;
			}

			ImGui::Selectable(label.c_str());
			RenderContextMenu(entry.path());
		}
	}

	void CAssetsBrowserWidget::RenderContextMenu(const std::filesystem::path& _path)
	{
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Open in Explorer"))
			{
				const std::string command = "explorer.exe /select,\"" + _path.string() + "\"";
				system(command.c_str());
			}

			const engine::assets::EAssetType asset_type = engine::game::CGame::Get().GetAssetsManager().DetectAssetType(_path);

			if (asset_type == assets::EAssetType::Texture)
			{
				if (ImGui::MenuItem("Generate Tileset"))
				{
					m_OpenGenerateTilesetPopup = true;
					m_PendingTilesetTexturePath = _path.string();
				}
			}

			if (engine::assets::STileset::IsTilesetFile(_path))
			{
				if (ImGui::MenuItem("Reload Tileset"))
				{
					engine::game::CGame::Get().GetTilesetManager().LoadTileset(_path);
				}

				if (ImGui::MenuItem("Delete Tileset"))
				{
					const std::string tileset_name = _path.stem().string();
					if (!engine::game::CGame::Get().GetTilesetManager().RemoveTileset(tileset_name))
					{
						CEditorManager::ShowNotification(notifications::ENotificationType::Error, "Failed to delete tileset.");
					}
					else
					{
						CEditorManager::ShowNotification(notifications::ENotificationType::Success, "Tileset deleted successfully.");
					}
				}
			}

			ImGui::EndPopup();
		}
	}

	bool CAssetsBrowserWidget::MatchesFilter(std::string_view _name) const
	{
		if (m_SearchFilter.empty())
		{
			return true;
		}

		const auto it = std::ranges::search(_name, m_SearchFilter, [](unsigned char _a, unsigned char _b)
			{
				return std::tolower(_a) == std::tolower(_b);
			});

		return !it.empty();
	}

	bool CAssetsBrowserWidget::DirectoryContentMatchesFilter(const std::filesystem::path& _path) const
	{
		for (const auto& entry : std::filesystem::recursive_directory_iterator(_path))
		{
			if (MatchesFilter(entry.path().filename().string()))
			{
				return true;
			}
		}

		return false;
	}

	void CAssetsBrowserWidget::RenderGenerateTilesetPopup()
	{
		if (m_OpenGenerateTilesetPopup)
		{
			ImGui::OpenPopup("Create Tileset");
			m_TilesetTileSize = { 0, 0 };
			m_TilesetMargin = { 0, 0 };
			m_TilesetSpacing = { 0, 0 };

			m_OpenGenerateTilesetPopup = false;
		}

		if (ImGui::BeginPopupModal("Create Tileset", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			const std::filesystem::path texture_path = std::filesystem::relative(m_PendingTilesetTexturePath, filesystem::Assets());
			const std::string texture_key = texture_path.string();

			if (!engine::game::CGame::Get().GetAssetsManager().Contains(texture_key))
			{
				CEditorManager::ShowNotification(notifications::ENotificationType::Error, "Texture not loaded in assets manager");
				ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
				return;
			}

			ImGui::TextDisabled("Generating tileset for texture %s", texture_key.c_str());

			ImGui::InputInt2("Tile Size", &m_TilesetTileSize.x);
			ImGui::InputInt2("Margin", &m_TilesetMargin.x);
			ImGui::InputInt2("Spacing", &m_TilesetSpacing.x);

			if (ImGui::Button("Create"))
			{
				engine::assets::STileset tileset;
				tileset.TexturePath = texture_path.string();
				tileset.Texture = engine::game::CGame::Get().GetAssetsManager().Get<sf::Texture>(texture_path.string());
				tileset.TileSize = m_TilesetTileSize;
				tileset.Margin = m_TilesetMargin;
				tileset.Spacing = m_TilesetSpacing;
				tileset.DetermineColumnsAndRows();

				if (!tileset.IsValid())
				{
					CEditorManager::ShowNotification(notifications::ENotificationType::Error, "Tileset is not valid.");
				}
				else
				{
					const std::string tileset_name = texture_path.stem().string();

					if (!engine::game::CGame::Get().GetTilesetManager().SaveTileset(tileset_name, tileset))
					{
						CEditorManager::ShowNotification(notifications::ENotificationType::Error, "Failed to save tileset.");
					}
					else
					{
						CEditorManager::ShowNotification(notifications::ENotificationType::Success, "Tileset created successfully.");
					}
				}

				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}
}

#endif