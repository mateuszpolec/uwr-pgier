#if defined(DEBUG)
#include "LevelEditorWidget.hpp"

#include <algorithm>
#include <cstdint>
#include <format>
#include <utility>

#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <imgui/SFML/imgui-SFML.h>
#include <SFML/Graphics/RenderWindow.hpp>

#include "EditorManager.hpp"
#include "notifications/NotificationType.hpp"
#include "engine/game/Game.hpp"
#include "engine/assets/TilesetManager.hpp"
#include "engine/ecs/PrefabManager.hpp"
#include "engine/level/LevelManager.hpp"
#include "engine/ecs/systems/PhysicsSystem.hpp"

namespace engine::editor
{
	namespace
	{
		sf::IntRect MakeSelectionRect(sf::Vector2i _a, sf::Vector2i _b)
		{
			const sf::Vector2i min{ std::min(_a.x, _b.x), std::min(_a.y, _b.y) };
			const sf::Vector2i max{ std::max(_a.x, _b.x), std::max(_a.y, _b.y) };
			return { min, { max.x - min.x + 1, max.y - min.y + 1 } };
		}

		engine::level::SLevelInfo MakeDefaultLevelInfo()
		{
			engine::level::SLevelInfo info;
			info.Name = "new_level";
			info.TileSize = { 32, 32 };
			info.Layers = {
				{ "Ground", engine::level::ELayerType::Tiles },
				{ "Decoration", engine::level::ELayerType::Tiles },
				{ "Objects", engine::level::ELayerType::Objects }
			};
			return info;
		}
	}

	CLevelEditorWidget::~CLevelEditorWidget()
	{
		engine::level::CLevelManager& level = engine::game::CGame::Get().GetLevelManager();

		level.DebugCollidersLayer = engine::level::CLevelManager::DEBUG_COLLIDERS_NONE;
	}

	bool CLevelEditorWidget::IsEditingActive() const
	{
		const engine::level::CLevelManager& level = engine::game::CGame::Get().GetLevelManager();
		return m_Visible && level.HasLevel() && !m_Layer.empty();
	}

	sf::Vector2i CLevelEditorWidget::PixelToTile(sf::Vector2i _pixel) const
	{
		auto& game = engine::game::CGame::Get();
		return game.GetLevelManager().WorldToTile(game.GetCameraManager().ScreenToWorld(_pixel));
	}

	void CLevelEditorWidget::EnsureValidLayer()
	{
		const auto& level = engine::game::CGame::Get().GetLevelManager();
		if (!level.HasLevel())
		{
			m_Layer.clear();
			return;
		}

		const auto& layers = level.GetInfo().Layers;
		if (layers.empty())
		{
			m_Layer.clear();
		}
		else if (level.GetInfo().FindLayerIndex(m_Layer) < 0)
		{
			m_Layer = layers.front().Name;
		}
	}

	void CLevelEditorWidget::Frame(float _dt)
	{
		EnsureValidLayer();

		if (!IsEditingActive())
		{
			m_HoveredTile.reset();
			m_Painting = false;
			return;
		}

		ProcessMouse();
		UpdateDebugColliders();
	}

	void CLevelEditorWidget::ProcessMouse()
	{
		const ImGuiIO& io = ImGui::GetIO();
		const sf::Vector2i pixel{ static_cast<int>(io.MousePos.x), static_cast<int>(io.MousePos.y) };
		const bool over_ui = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) || ImGui::IsAnyItemHovered() || ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId);

		m_HoveredTile = PixelToTile(pixel);

		engine::level::CLevelManager& level = engine::game::CGame::Get().GetLevelManager();

		const bool lmb_clicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
		const bool rmb_clicked = ImGui::IsMouseClicked(ImGuiMouseButton_Right);
		const bool lmb_down = ImGui::IsMouseDown(ImGuiMouseButton_Left);
		const bool lmb_released = ImGui::IsMouseReleased(ImGuiMouseButton_Left);
		const bool rmb_released = ImGui::IsMouseReleased(ImGuiMouseButton_Right);

		if (m_AutoCreateChunks && !over_ui && lmb_clicked && !level.FindChunk(level.TileToChunk(*m_HoveredTile)))
		{
			level.GetOrCreateChunk(level.TileToChunk(*m_HoveredTile)).MarkDirty();
		}

		if (m_Tool == ELevelEditorTool::Fill && m_BrushMode != EBrushMode::Prefab)
		{
			if (!m_SelectionStart && !over_ui && (lmb_clicked || rmb_clicked))
			{
				m_SelectionStart = m_HoveredTile;
				m_SelectionEnd = m_HoveredTile;
				m_EraseSelection = rmb_clicked;
			}
			else if (m_SelectionStart)
			{
				m_SelectionEnd = m_HoveredTile;

				if (lmb_released || rmb_released)
				{
					ApplySelection(m_EraseSelection);
					m_SelectionStart.reset();
					m_SelectionEnd.reset();
					m_EraseSelection = false;
				}
			}
			return;
		}

		if (m_BrushMode == EBrushMode::Prefab)
		{
			if (!over_ui && lmb_clicked)
			{
				ApplyBrush(*m_HoveredTile);
			}
			return;
		}

		if (!over_ui && lmb_clicked)
		{
			m_Painting = true;
		}
		if (!lmb_down)
		{
			m_Painting = false;
		}
		if (m_Painting && !over_ui)
		{
			ApplyBrush(*m_HoveredTile);
		}
	}

	void CLevelEditorWidget::UpdateDebugColliders()
	{
		using engine::level::CLevelManager;
		CLevelManager& level = engine::game::CGame::Get().GetLevelManager();

		if (!m_ShowColliders)
		{
			level.DebugCollidersLayer = CLevelManager::DEBUG_COLLIDERS_NONE;
		}
		else if (m_ShowAllLayersColliders)
		{
			level.DebugCollidersLayer = CLevelManager::DEBUG_COLLIDERS_ALL;
		}
		else
		{
			level.DebugCollidersLayer = level.GetInfo().FindLayerIndex(m_Layer);
		}
	}

	void CLevelEditorWidget::ApplyBrush(sf::Vector2i _tile)
	{
		engine::level::CLevelManager& level = engine::game::CGame::Get().GetLevelManager();
		const bool erase = m_Tool == ELevelEditorTool::Erase;

		switch (m_BrushMode)
		{
		case EBrushMode::Collider:
		{
			level.SetCollider(m_Layer, _tile, !erase);
			break;
		}
		case EBrushMode::Tile:
		{
			if (erase)
			{
				level.SetTile(m_Layer, _tile, {});
			}
			else if (m_TileBrush.IsValid())
			{
				level.SetTile(m_Layer, _tile, m_TileBrush);
			}
			break;
		}
		case EBrushMode::Prefab:
		{
			if (!erase && !m_PrefabBrush.empty())
			{
				level.AddPrefabInstance(m_Layer, level.TileToWorld(_tile), m_PrefabBrush, m_PresetBrush);
			}
			break;
		}
		}
	}

	void CLevelEditorWidget::ApplySelection(bool _erase)
	{
		if (!m_SelectionStart || !m_SelectionEnd)
		{
			return;
		}

		const sf::IntRect rect = MakeSelectionRect(*m_SelectionStart, *m_SelectionEnd);
		engine::level::CLevelManager& level = engine::game::CGame::Get().GetLevelManager();

		if (m_BrushMode == EBrushMode::Collider)
		{
			level.FillColliders(m_Layer, rect, !_erase);
		}
		else
		{
			level.FillTiles(m_Layer, rect, _erase ? engine::level::STile{} : m_TileBrush);
		}
	}

	void CLevelEditorWidget::Render()
	{
		if (ImGui::Begin(GetName().data(), &m_Visible, ImGuiWindowFlags_MenuBar))
		{
			RenderMenuBar();

			engine::level::CLevelManager& level = engine::game::CGame::Get().GetLevelManager();

			RenderLevelSelector();

			if (level.HasLevel())
			{
				ImGui::BeginChild("Tools", { 260.f, 0.f }, ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
				RenderToolbar();
				ImGui::Separator();
				RenderLayerList();
				ImGui::EndChild();

				ImGui::SameLine();

				ImGui::BeginChild("Palette", { 0.f, 0.f }, ImGuiChildFlags_Borders);
				switch (m_BrushMode)
				{
					case EBrushMode::Tile:
					{
						RenderTilePalette();
						break;
					}
					case EBrushMode::Prefab:
					{
						RenderPrefabPalette();
						break;
					}
					case EBrushMode::Collider:
					{
						ImGui::TextWrapped("Collider brush paints a collision mask on layer '%s'. Use Place / Erase / Fill like with tiles.", m_Layer.c_str());
						break;
					}
				}
				ImGui::EndChild();
			}

			RenderCreateLevelPopup();
			RenderDeleteLevelPopup();
		}

		ImGui::End();

		RenderOverlay();
	}

	void CLevelEditorWidget::RenderMenuBar()
	{
		if (!ImGui::BeginMenuBar())
		{
			return;
		}

		engine::level::CLevelManager& level = engine::game::CGame::Get().GetLevelManager();
		const bool has_level = level.HasLevel();

		if (ImGui::BeginMenu("Level"))
		{
			if (ImGui::MenuItem("New..."))
			{
				m_NewLevel = MakeDefaultLevelInfo();
				m_OpenCreatePopup = true;
			}

			if (ImGui::MenuItem("Save dirty chunks", nullptr, false, has_level))
			{
				level.SaveDirtyChunks();
				CEditorManager::ShowNotification(notifications::ENotificationType::Info, "Level saved");
			}

			if (ImGui::MenuItem("Save all loaded chunks", nullptr, false, has_level))
			{
				level.SaveAll();
				CEditorManager::ShowNotification(notifications::ENotificationType::Info, "All chunks saved");
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Unload", nullptr, false, has_level))
			{
				level.UnloadLevel();
			}

			if (ImGui::MenuItem("Delete...", nullptr, false, has_level))
			{
				m_LevelToDelete = level.GetInfo().Name;
				m_OpenDeletePopup = true;
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			ImGui::MenuItem("Show colliders", nullptr, &m_ShowColliders);
			ImGui::MenuItem("Colliders of all layers", nullptr, &m_ShowAllLayersColliders, m_ShowColliders);
			ImGui::Separator();
			ImGui::MenuItem("Auto-create chunks on click", nullptr, &m_AutoCreateChunks);
			if (has_level)
			{
				ImGui::MenuItem("Streaming", nullptr, &level.StreamingEnabled);
			}
			ImGui::EndMenu();
		}

		ImGui::Separator();
		{
			engine::ecs::systems::CPhysicsSystem& physics = *engine::game::CGame::Get().GetSystemManager().FindSystem<engine::ecs::systems::CPhysicsSystem>(engine::ecs::ESystemType::Physics);
			ImGui::MenuItem("Show physics bodies", nullptr, &physics.DebugDraw);
			if (ImGui::BeginMenu("Broad phase"))
			{
				using engine::physics::ECollisionSolver;
				for (const auto type : { ECollisionSolver::NoCollision, ECollisionSolver::BruteForce, ECollisionSolver::Quadtree })
				{
					if (ImGui::MenuItem(engine::physics::ToString(type).data(), nullptr, physics.GetSolverType() == type))
					{
						physics.SetSolver(type);
					}
				}
				ImGui::EndMenu();
			}

			ImGui::MenuItem("Show broad phase", nullptr, &physics.DebugDrawSolver);
		}

		ImGui::EndMenuBar();
	}

	void CLevelEditorWidget::RenderLevelSelector()
	{
		engine::level::CLevelManager& level = engine::game::CGame::Get().GetLevelManager();
		const std::string current = level.HasLevel() ? level.GetInfo().Name : std::string{};

		if (ImGui::BeginCombo("Level", current.empty() ? "<none>" : current.c_str()))
		{
			m_AvailableLevels = engine::level::CLevelManager::ListLevels();

			for (const std::string& name : m_AvailableLevels)
			{
				if (ImGui::Selectable(name.c_str(), name == current) && name != current)
				{
					if (level.LoadLevel(name))
					{
						m_Layer.clear();
						CEditorManager::ShowNotification(notifications::ENotificationType::Info, std::format("Level '{}' loaded", name));
					}
					else
					{
						CEditorManager::ShowNotification(notifications::ENotificationType::Error, std::format("Failed to load level '{}'", name));
					}
				}
			}

			if (m_AvailableLevels.empty())
			{
				ImGui::TextDisabled("No levels found in assets/levels");
			}

			ImGui::EndCombo();
		}

		if (level.HasLevel())
		{
			ImGui::SameLine();
			ImGui::TextDisabled("chunks: %zu | pending: %zu", level.GetChunks().size(), level.GetPendingLoadsCount());
		}
	}

	void CLevelEditorWidget::RenderToolbar()
	{
		engine::level::CLevelManager& level = engine::game::CGame::Get().GetLevelManager();

		ImGui::SeparatorText("Tool");
		int tool = static_cast<int>(m_Tool);
		ImGui::RadioButton("Place", &tool, static_cast<int>(ELevelEditorTool::Place)); ImGui::SameLine();
		ImGui::RadioButton("Erase", &tool, static_cast<int>(ELevelEditorTool::Erase)); ImGui::SameLine();
		ImGui::RadioButton("Fill", &tool, static_cast<int>(ELevelEditorTool::Fill));
		m_Tool = static_cast<ELevelEditorTool>(tool);

		if (m_Tool == ELevelEditorTool::Fill)
		{
			ImGui::TextDisabled("LMB drag: fill | RMB drag: erase");
		}

		ImGui::SeparatorText("Brush");
		int brush = static_cast<int>(m_BrushMode);
		ImGui::RadioButton("Tile", &brush, static_cast<int>(EBrushMode::Tile)); ImGui::SameLine();
		ImGui::RadioButton("Collider", &brush, static_cast<int>(EBrushMode::Collider)); ImGui::SameLine();
		ImGui::RadioButton("Prefab", &brush, static_cast<int>(EBrushMode::Prefab));
		m_BrushMode = static_cast<EBrushMode>(brush);

		ImGui::SeparatorText("Cursor");
		if (m_HoveredTile)
		{
			const engine::level::SChunkCoordinates chunk_coord = level.TileToChunk(*m_HoveredTile);
			const bool chunk_loaded = level.FindChunk(chunk_coord) != nullptr;

			ImGui::Text("Tile: (%d, %d)", m_HoveredTile->x, m_HoveredTile->y);
			ImGui::Text("Chunk: (%d, %d) %s", chunk_coord.X, chunk_coord.Y, chunk_loaded ? "" : "[not loaded]");
			if (chunk_loaded && level.IsColliderAt(m_Layer, *m_HoveredTile))
			{
				ImGui::TextColored(ImVec4(1.f, 0.4f, 0.4f, 1.f), "Collider on '%s'", m_Layer.c_str());
			}
		}
	}

	void CLevelEditorWidget::RenderLayerList()
	{
		engine::level::CLevelManager& level = engine::game::CGame::Get().GetLevelManager();
		const engine::level::SLevelInfo& info = level.GetInfo();

		ImGui::SeparatorText("Layers");

		if (ImGui::SmallButton("+ Add layer"))
		{
			m_LayerActionName = std::format("Layer {}", info.Layers.size() + 1);
			m_LayerActionType = engine::level::ELayerType::Tiles;
			m_LayerActionIndex = info.Layers.size();
			m_OpenAddLayerPopup = true;
		}
		ImGui::SameLine();
		ImGui::TextDisabled("(RMB on layer for actions, drag to reorder)");

		// Snapshot - the list may be mutated by actions below.
		const size_t layer_count = info.Layers.size();
		std::optional<std::pair<size_t, size_t>> pending_move;

		for (size_t i = 0; i < layer_count; ++i)
		{
			const engine::level::SLayerInfo& layer = info.Layers[i];
			ImGui::PushID(static_cast<int>(i));

			bool visible = level.IsLayerVisible(i);
			if (ImGui::Checkbox("##visible", &visible))
			{
				level.SetLayerVisible(i, visible);
			}
			ImGui::SameLine();

			const std::string label = std::format("{} [{}]", layer.Name, engine::level::ToString(layer.Type));
			if (ImGui::Selectable(label.c_str(), layer.Name == m_Layer))
			{
				m_Layer = layer.Name;
			}

			// Drag & drop reorder.
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoHoldToOpenOthers))
			{
				ImGui::SetDragDropPayload("LEVEL_LAYER", &i, sizeof(size_t));
				ImGui::Text("Move '%s'", layer.Name.c_str());
				ImGui::EndDragDropSource();
			}
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("LEVEL_LAYER"))
				{
					const size_t from = *static_cast<const size_t*>(payload->Data);
					pending_move = std::make_pair(from, i);
				}
				ImGui::EndDragDropTarget();
			}

			RenderLayerContextMenu(i);

			ImGui::PopID();
		}

		if (pending_move && pending_move->first != pending_move->second)
		{
			level.MoveLayer(pending_move->first, pending_move->second);
		}

		RenderAddLayerPopup();
		RenderRenameLayerPopup();
		RenderRemoveLayerPopup();
	}

	void CLevelEditorWidget::RenderLayerContextMenu(size_t _layer_index)
	{
		engine::level::CLevelManager& level = engine::game::CGame::Get().GetLevelManager();
		const engine::level::SLevelInfo& info = level.GetInfo();
		const size_t count = info.Layers.size();

		if (!ImGui::BeginPopupContextItem("##layer_ctx"))
		{
			return;
		}

		const engine::level::SLayerInfo& layer = info.Layers[_layer_index];
		ImGui::TextDisabled("%s", layer.Name.c_str());
		ImGui::Separator();

		if (ImGui::MenuItem("Move up", nullptr, false, _layer_index > 0))
		{
			level.MoveLayer(_layer_index, _layer_index - 1);
		}
		if (ImGui::MenuItem("Move down", nullptr, false, _layer_index + 1 < count))
		{
			level.MoveLayer(_layer_index, _layer_index + 1);
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Insert layer above..."))
		{
			m_LayerActionName = std::format("Layer {}", count + 1);
			m_LayerActionType = engine::level::ELayerType::Tiles;
			m_LayerActionIndex = _layer_index;
			m_OpenAddLayerPopup = true;
		}
		if (ImGui::MenuItem("Insert layer below..."))
		{
			m_LayerActionName = std::format("Layer {}", count + 1);
			m_LayerActionType = engine::level::ELayerType::Tiles;
			m_LayerActionIndex = _layer_index + 1;
			m_OpenAddLayerPopup = true;
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Rename..."))
		{
			m_LayerActionIndex = _layer_index;
			m_LayerActionName = layer.Name;
			m_OpenRenameLayerPopup = true;
		}

		if (ImGui::BeginMenu("Type"))
		{
			for (const auto type : { engine::level::ELayerType::Tiles, engine::level::ELayerType::Objects })
			{
				if (ImGui::MenuItem(engine::level::ToString(type).data(), nullptr, layer.Type == type))
				{
					level.SetLayerType(_layer_index, type);
				}
			}
			ImGui::EndMenu();
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Remove...", nullptr, false, count > 1))
		{
			m_LayerActionIndex = _layer_index;
			m_LayerActionName = layer.Name;
			m_OpenRemoveLayerPopup = true;
		}

		ImGui::EndPopup();
	}

	void CLevelEditorWidget::RenderAddLayerPopup()
	{
		constexpr const char* POPUP = "Add layer";

		if (m_OpenAddLayerPopup)
		{
			ImGui::OpenPopup(POPUP);
			m_OpenAddLayerPopup = false;
		}

		if (!ImGui::BeginPopupModal(POPUP, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			return;
		}

		engine::level::CLevelManager& level = engine::game::CGame::Get().GetLevelManager();

		ImGui::InputText("Name", &m_LayerActionName);

		int type = static_cast<int>(m_LayerActionType);
		ImGui::RadioButton("Tiles", &type, static_cast<int>(engine::level::ELayerType::Tiles)); ImGui::SameLine();
		ImGui::RadioButton("Objects", &type, static_cast<int>(engine::level::ELayerType::Objects));
		m_LayerActionType = static_cast<engine::level::ELayerType>(type);

		const bool name_taken = level.GetInfo().FindLayerIndex(m_LayerActionName) >= 0;
		if (name_taken)
		{
			ImGui::TextColored(ImVec4(1.f, 0.4f, 0.4f, 1.f), "Layer with this name already exists");
		}

		ImGui::BeginDisabled(m_LayerActionName.empty() || name_taken);
		if (ImGui::Button("Add", ImVec2(120.f, 0.f)))
		{
			if (level.AddLayer(m_LayerActionName, m_LayerActionType, m_LayerActionIndex))
			{
				m_Layer = m_LayerActionName;
				CEditorManager::ShowNotification(notifications::ENotificationType::Info, std::format("Layer '{}' added", m_LayerActionName));
			}
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndDisabled();

		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120.f, 0.f)))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	void CLevelEditorWidget::RenderRenameLayerPopup()
	{
		constexpr const char* POPUP = "Rename layer";

		if (m_OpenRenameLayerPopup)
		{
			ImGui::OpenPopup(POPUP);
			m_OpenRenameLayerPopup = false;
		}

		if (!ImGui::BeginPopupModal(POPUP, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			return;
		}

		engine::level::CLevelManager& level = engine::game::CGame::Get().GetLevelManager();
		const engine::level::SLevelInfo& info = level.GetInfo();

		if (m_LayerActionIndex >= info.Layers.size())
		{
			ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
			return;
		}

		const std::string& old_name = info.Layers[m_LayerActionIndex].Name;
		ImGui::InputText("New name", &m_LayerActionName);

		const bool unchanged = m_LayerActionName == old_name;
		const bool name_taken = !unchanged && info.FindLayerIndex(m_LayerActionName) >= 0;
		if (name_taken)
		{
			ImGui::TextColored(ImVec4(1.f, 0.4f, 0.4f, 1.f), "Layer with this name already exists");
		}

		ImGui::BeginDisabled(m_LayerActionName.empty() || unchanged || name_taken);
		if (ImGui::Button("Rename", ImVec2(120.f, 0.f)))
		{
			const bool was_selected = m_Layer == old_name;
			if (level.RenameLayer(m_LayerActionIndex, m_LayerActionName))
			{
				if (was_selected)
				{
					m_Layer = m_LayerActionName;
				}
				CEditorManager::ShowNotification(notifications::ENotificationType::Info, "Layer renamed");
			}
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndDisabled();

		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120.f, 0.f)))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	void CLevelEditorWidget::RenderRemoveLayerPopup()
	{
		constexpr const char* POPUP = "Remove layer";

		if (m_OpenRemoveLayerPopup)
		{
			ImGui::OpenPopup(POPUP);
			m_OpenRemoveLayerPopup = false;
		}

		if (!ImGui::BeginPopupModal(POPUP, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			return;
		}

		engine::level::CLevelManager& level = engine::game::CGame::Get().GetLevelManager();

		ImGui::Text("Remove layer '%s'?", m_LayerActionName.c_str());
		ImGui::TextColored(ImVec4(1.f, 0.4f, 0.4f, 1.f), "All tiles, colliders and prefabs on this layer will be deleted\nfrom every chunk of this level. This cannot be undone.");
		ImGui::Separator();

		if (ImGui::Button("Remove", ImVec2(120.f, 0.f)))
		{
			if (level.RemoveLayer(m_LayerActionIndex))
			{
				if (m_Layer == m_LayerActionName)
				{
					m_Layer.clear();
				}
				CEditorManager::ShowNotification(notifications::ENotificationType::Info, std::format("Layer '{}' removed", m_LayerActionName));
			}
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120.f, 0.f)))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	void CLevelEditorWidget::RenderTilePalette()
	{
		constexpr float PREVIEW_SIZE = 32.f;

		const auto& tilesets = engine::game::CGame::Get().GetTilesetManager();

		if (ImGui::BeginCombo("Tileset", m_TileBrush.Tileset.c_str()))
		{
			for (const auto& [name, tileset] : tilesets.GetTilesets())
			{
				if (ImGui::Selectable(name.c_str(), name == m_TileBrush.Tileset))
				{
					m_TileBrush.Tileset = name;
					m_TileBrush.Index = 0;
				}
			}
			ImGui::EndCombo();
		}

		if (m_TileBrush.Tileset.empty())
		{
			ImGui::TextDisabled("Select a valid tileset.");
			return;
		}

		const engine::assets::STileset* tileset = tilesets.FindTileset(m_TileBrush.Tileset);
		if (!tileset || !tileset->IsValid())
		{
			ImGui::TextDisabled("Select a valid tileset.");
			return;
		}

		ImGui::BeginChild("TilePalette", ImVec2(0, 0), ImGuiChildFlags_Borders);

		const int columns = std::max(1, static_cast<int>(ImGui::GetContentRegionAvail().x / (PREVIEW_SIZE + 10.f)));
		const sf::Texture& texture = tileset->Texture.Get();
		const sf::Vector2f texture_size = static_cast<sf::Vector2f>(texture.getSize());
		const ImTextureID texture_id = static_cast<ImTextureID>(static_cast<std::uintptr_t>(texture.getNativeHandle()));

		for (int index = 0; index < tileset->GetTileCount(); ++index)
		{
			const std::optional<sf::IntRect> rect = tileset->GetTileRect(index);
			if (!rect)
			{
				continue;
			}

			const ImVec2 uv0{ rect->position.x / texture_size.x, rect->position.y / texture_size.y };
			const ImVec2 uv1{ (rect->position.x + rect->size.x) / texture_size.x, (rect->position.y + rect->size.y) / texture_size.y };

			ImGui::PushID(index);
			const bool selected = m_TileBrush.Index == index;
			if (selected)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 1.f, 1.f));
			}

			if (ImGui::ImageButton("tile", texture_id, ImVec2(PREVIEW_SIZE, PREVIEW_SIZE), uv0, uv1))
			{
				m_TileBrush.Index = index;
			}

			if (selected)
			{
				ImGui::PopStyleColor();
			}
			ImGui::PopID();

			if ((index + 1) % columns != 0)
			{
				ImGui::SameLine();
			}
		}

		ImGui::EndChild();
	}

	void CLevelEditorWidget::RenderPrefabPalette()
	{
		const auto& prefabs = engine::game::CGame::Get().GetPrefabManager();

		ImGui::InputText("Prefab", &m_PrefabBrush);
		ImGui::InputText("Preset", &m_PresetBrush);

		if (!m_PrefabBrush.empty() && !prefabs.HasPrefab(m_PrefabBrush))
		{
			ImGui::TextColored(ImVec4(1.f, 0.4f, 0.4f, 1.f), "Unknown prefab '%s'", m_PrefabBrush.c_str());
		}
		else if (!m_PresetBrush.empty() && !prefabs.HasPreset(m_PrefabBrush, m_PresetBrush))
		{
			ImGui::TextColored(ImVec4(1.f, 0.4f, 0.4f, 1.f), "Unknown preset '%s'", m_PresetBrush.c_str());
		}

		ImGui::TextWrapped("LMB places the prefab at the hovered tile on layer '%s'. Remove instances in Scene View.", m_Layer.c_str());
	}

	void CLevelEditorWidget::RenderOverlay()
	{
		if (!IsEditingActive())
		{
			return;
		}

		auto& game = engine::game::CGame::Get();
		const engine::level::CLevelManager& level = game.GetLevelManager();
		const sf::RenderWindow& window = game.GetWindow();
		ImDrawList* draw = ImGui::GetBackgroundDrawList();

		const auto to_screen = [&](sf::Vector2f _world)
		{
				const sf::Vector2i p = engine::game::CGame::Get().GetCameraManager().WorldToScreen(_world);
				return ImVec2(static_cast<float>(p.x), static_cast<float>(p.y));
		};

		const auto tile_rect_to_screen = [&](sf::Vector2i _min_tile, sf::Vector2i _max_tile)
		{
				return std::pair{ to_screen(level.TileToWorld(_min_tile)), to_screen(level.TileToWorld({ _max_tile.x + 1, _max_tile.y + 1 })) };
		};

		for (const auto& [coord, chunk] : level.GetChunks())
		{
			const sf::FloatRect b = chunk->GetWorldBounds();
			draw->AddRect(to_screen(b.position), to_screen(b.position + b.size), IM_COL32(255, 255, 255, 50));
		}

		if (m_HoveredTile)
		{
			const engine::level::SChunkCoordinates chunk_coord = level.TileToChunk(*m_HoveredTile);
			const auto [a, b] = tile_rect_to_screen(*m_HoveredTile, *m_HoveredTile);
			const ImU32 color = level.FindChunk(chunk_coord) ? IM_COL32(255, 255, 0, 200) : IM_COL32(255, 128, 0, 200);
			draw->AddRect(a, b, color, 0.f, 0, 2.f);
		}

		if (m_SelectionStart && m_SelectionEnd)
		{
			const sf::IntRect rect = MakeSelectionRect(*m_SelectionStart, *m_SelectionEnd);
			const auto [a, b] = tile_rect_to_screen(rect.position, { rect.position.x + rect.size.x - 1, rect.position.y + rect.size.y - 1 });
			const ImU32 color = m_EraseSelection ? IM_COL32(255, 80, 80, 255) : IM_COL32(0, 150, 255, 255);
			draw->AddRectFilled(a, b, (color & 0x00FFFFFF) | (60u << 24));
			draw->AddRect(a, b, color);
		}
	}

	void CLevelEditorWidget::RenderCreateLevelPopup()
	{
		constexpr const char* POPUP_ID = "Create level";

		if (m_OpenCreatePopup)
		{
			ImGui::OpenPopup(POPUP_ID);
			m_OpenCreatePopup = false;
		}

		if (!ImGui::BeginPopupModal(POPUP_ID, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			return;
		}

		ImGui::InputText("Name", &m_NewLevel.Name);
		ImGui::InputInt2("Tile size", &m_NewLevel.TileSize.x);
		ImGui::SeparatorText("Layers");

		int to_remove = -1;
		for (size_t i = 0; i < m_NewLevel.Layers.size(); ++i)
		{
			engine::level::SLayerInfo& layer = m_NewLevel.Layers[i];
			ImGui::PushID(static_cast<int>(i));

			ImGui::SetNextItemWidth(140.f);
			ImGui::InputText("##name", &layer.Name);
			ImGui::SameLine();

			int type = static_cast<int>(layer.Type);
			ImGui::SetNextItemWidth(90.f);
			if (ImGui::Combo("##type", &type, "Tiles\0Objects\0"))
			{
				layer.Type = static_cast<engine::level::ELayerType>(type);
			}
			ImGui::SameLine();

			if (ImGui::SmallButton("X"))
			{
				to_remove = static_cast<int>(i);
			}

			ImGui::PopID();
		}

		if (to_remove >= 0)
		{
			m_NewLevel.Layers.erase(m_NewLevel.Layers.begin() + to_remove);
		}

		ImGui::SetNextItemWidth(140.f);
		ImGui::InputText("##new_layer", &m_NewLayerName);
		ImGui::SameLine();
		if (ImGui::Button("Add layer") && !m_NewLayerName.empty())
		{
			m_NewLevel.Layers.push_back({ m_NewLayerName, engine::level::ELayerType::Tiles });
			m_NewLayerName.clear();
		}

		ImGui::Separator();

		const std::vector<std::string> existing_levels = engine::level::CLevelManager::ListLevels();
		const bool name_taken = std::find(existing_levels.begin(), existing_levels.end(), m_NewLevel.Name) != existing_levels.end();
		const bool valid = !m_NewLevel.Name.empty() && !m_NewLevel.Layers.empty() && m_NewLevel.TileSize.x > 0 && m_NewLevel.TileSize.y > 0 && !name_taken;

		if (name_taken)
		{
			ImGui::TextColored(ImVec4(1.f, 0.4f, 0.4f, 1.f), "Level '%s' already exists.", m_NewLevel.Name.c_str());
		}

		ImGui::BeginDisabled(!valid);
		if (ImGui::Button("Create", { 120.f, 0.f }))
		{
			engine::level::CLevelManager& level = engine::game::CGame::Get().GetLevelManager();
			if (level.CreateLevel(m_NewLevel))
			{
				m_Layer.clear();
				CEditorManager::ShowNotification(notifications::ENotificationType::Info, std::format("Level '{}' created", m_NewLevel.Name));
			}
			else
			{
				CEditorManager::ShowNotification(notifications::ENotificationType::Error, "Failed to create level");
			}
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndDisabled();

		ImGui::SameLine();
		if (ImGui::Button("Cancel", { 120.f, 0.f }))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	void CLevelEditorWidget::RenderDeleteLevelPopup()
	{
		constexpr const char* POPUP_ID = "Delete level";

		if (m_OpenDeletePopup)
		{
			ImGui::OpenPopup(POPUP_ID);
			m_OpenDeletePopup = false;
		}

		if (!ImGui::BeginPopupModal(POPUP_ID, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			return;
		}

		ImGui::Text("Delete level '%s' and all its chunks from disk?", m_LevelToDelete.c_str());
		ImGui::TextColored(ImVec4(1.f, 0.4f, 0.4f, 1.f), "This cannot be undone.");
		ImGui::Separator();

		if (ImGui::Button("Delete", { 120.f, 0.f }))
		{
			engine::level::CLevelManager& level = engine::game::CGame::Get().GetLevelManager();
			if (level.DeleteLevel(m_LevelToDelete))
			{
				m_Layer.clear();
				CEditorManager::ShowNotification(notifications::ENotificationType::Info, std::format("Level '{}' deleted", m_LevelToDelete));
			}
			else
			{
				CEditorManager::ShowNotification(notifications::ENotificationType::Error, "Failed to delete level");
			}
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();
		if (ImGui::Button("Cancel", { 120.f, 0.f }))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}
#endif