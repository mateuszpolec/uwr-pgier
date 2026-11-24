#if defined(DEBUG)
#pragma once

#include <optional>
#include <string>
#include <vector>

#include <SFML/System/Vector2.hpp>

#include "engine/level/LevelData.hpp"
#include "engine/level/LevelTypes.hpp"
#include "Widget.hpp"

namespace engine::editor
{
	///////////////////////////////////////////////////////////////////////////////////////
	enum class ELevelEditorTool
	{
		Place,
		Erase,
		Fill
	};

	enum class EBrushMode
	{
		Tile,
		Collider,
		Prefab
	};

	///////////////////////////////////////////////////////////////////////////////////////
	class CLevelEditorWidget : public IWidget
	{
	public:
		CLevelEditorWidget() = default;
		~CLevelEditorWidget();

		[[nodiscard]] std::string_view GetName() const override { return "Level Editor"; }
		[[nodiscard]] std::string_view GetMenuName() const override { return "Windows"; }

		void Frame(float _dt) override;
		void Render() override;

		[[nodiscard]] bool GetRenderInMenu() const override { return true; }

	private:

		void RenderMenuBar();
		void RenderLevelSelector();
		void RenderToolbar();
		void RenderTilePalette();
		void RenderPrefabPalette();
		void RenderOverlay();
		void RenderCreateLevelPopup();
		void RenderDeleteLevelPopup();

		void RenderLayerList();
		void RenderLayerContextMenu(size_t _layer_index);
		void RenderAddLayerPopup();
		void RenderRenameLayerPopup();
		void RenderRemoveLayerPopup();

		void ProcessMouse();
		void ApplyBrush(sf::Vector2i _tile);
		void ApplySelection(bool _erase);
		void UpdateDebugColliders();
		void EnsureValidLayer();

		[[nodiscard]] sf::Vector2i PixelToTile(sf::Vector2i _pixel) const;
		[[nodiscard]] bool IsEditingActive() const;

	private:

		ELevelEditorTool m_Tool = ELevelEditorTool::Place;
		EBrushMode m_BrushMode = EBrushMode::Tile;

		std::string m_Layer;
		engine::level::STile m_TileBrush;
		std::string m_PrefabBrush;
		std::string m_PresetBrush;

		bool m_ShowColliders = true;
		bool m_ShowAllLayersColliders = false;
		bool m_AutoCreateChunks = true;

		bool m_Painting = false;
		bool m_EraseSelection = false;
		std::optional<sf::Vector2i> m_HoveredTile;
		std::optional<sf::Vector2i> m_SelectionStart;
		std::optional<sf::Vector2i> m_SelectionEnd;

		float m_ColliderFriction = 0.f;

		bool m_OpenCreatePopup = false;
		bool m_OpenDeletePopup = false;
		engine::level::SLevelInfo m_NewLevel;
		std::string m_NewLayerName;
		std::string m_LevelToDelete;
		std::vector<std::string> m_AvailableLevels;

		bool m_OpenAddLayerPopup = false;
		bool m_OpenRenameLayerPopup = false;
		bool m_OpenRemoveLayerPopup = false;
		size_t m_LayerActionIndex = 0;
		std::string m_LayerActionName;
		engine::level::ELayerType m_LayerActionType = engine::level::ELayerType::Tiles;
	};
}
#endif