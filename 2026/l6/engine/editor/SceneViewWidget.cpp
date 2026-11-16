#if defined(DEBUG)

#include <format>
#include <optional>
#include <string>
#include <tuple>

#include <imgui/imgui.h>

#include "SceneViewWidget.hpp"
#include "EditorManager.hpp"
#include "notifications/NotificationType.hpp"
#include "engine/game/Game.hpp"
#include "engine/level/LevelManager.hpp"
#include "engine/ecs/components/Component_Transform.hpp"

namespace engine::editor
{
	void CSceneViewWidget::Render()
	{
		if (ImGui::Begin(GetName().data(), &m_Visible))
		{
			engine::level::CLevelManager& level = engine::game::CGame::Get().GetLevelManager();

			if (!level.HasLevel())
			{
				ImGui::TextDisabled("No level loaded. Use Level Editor to create or load one.");
			}
			else
			{
				RenderHeader();
				ImGui::Separator();

				if (ImGui::CollapsingHeader("Layers", ImGuiTreeNodeFlags_DefaultOpen))
				{
					RenderLayers();
				}

				if (ImGui::CollapsingHeader("Chunks", ImGuiTreeNodeFlags_DefaultOpen))
				{
					RenderChunks();
				}
			}
		}

		ImGui::End();
	}

	void CSceneViewWidget::RenderHeader()
	{
		engine::level::CLevelManager& level = engine::game::CGame::Get().GetLevelManager();
		engine::game::CGameSettings& settings = engine::game::CGame::Get().GetGameSettings();
		const engine::level::SLevelInfo& info = level.GetInfo();

		ImGui::Text("Level: %s", info.Name.c_str());
		ImGui::Text("Tile size: %dx%d | Streaming radius: %d", info.TileSize.x, info.TileSize.y, settings.GetStreamingRadius());
		ImGui::Text("Loaded chunks: %zu | Pending loads: %zu", level.GetChunks().size(), level.GetPendingLoadsCount());

		ImGui::Checkbox("Streaming", &level.StreamingEnabled);
		ImGui::SameLine();
		if (ImGui::Button("Save dirty chunks"))
		{
			level.SaveDirtyChunks();
			CEditorManager::ShowNotification(notifications::ENotificationType::Info, "Dirty chunks saved");
		}
	}

	void CSceneViewWidget::RenderLayers()
	{
		engine::level::CLevelManager& level = engine::game::CGame::Get().GetLevelManager();
		const engine::level::SLevelInfo& info = level.GetInfo();

		for (size_t i = 0; i < info.Layers.size(); ++i)
		{
			const engine::level::SLayerInfo& layer = info.Layers[i];

			ImGui::PushID(static_cast<int>(i));

			bool visible = level.IsLayerVisible(i);
			if (ImGui::Checkbox("##visible", &visible))
			{
				level.SetLayerVisible(i, visible);
			}
			ImGui::SameLine();
			ImGui::Text("%zu. %s [%s]", i, layer.Name.c_str(), engine::level::ToString(layer.Type).data());

			ImGui::PopID();
		}
	}

	void CSceneViewWidget::RenderChunks()
	{
		auto& game = engine::game::CGame::Get();
		engine::level::CLevelManager& level = game.GetLevelManager();
		auto& entities = game.GetEntityManager();

		std::optional<engine::level::SChunkCoordinates> chunk_to_remove;
		std::optional<std::tuple<engine::level::SChunkCoordinates, std::string, size_t>> prefab_to_remove;

		for (const auto& [coord, chunk] : level.GetChunks())
		{
			const engine::level::SChunkData& data = chunk->GetData();
			const std::string label = std::format("Chunk ({}, {}){}###chunk_{}_{}", coord.X, coord.Y, chunk->IsDirty() ? " *" : "", coord.X, coord.Y);

			if (!ImGui::TreeNode(label.c_str()))
			{
				continue;
			}

			const sf::FloatRect bounds = chunk->GetWorldBounds();
			ImGui::TextDisabled("World: (%.0f, %.0f) - (%.0f, %.0f)", bounds.position.x, bounds.position.y, bounds.position.x + bounds.size.x, bounds.position.y + bounds.size.y);

			if (ImGui::SmallButton("Remove chunk"))
			{
				chunk_to_remove = coord;
			}

			for (size_t layer_index = 0; layer_index < data.Layers.size(); ++layer_index)
			{
				const engine::level::SLayerData& layer = data.Layers[layer_index];
				const std::string layer_label = std::format("{} - tiles: {} | colliders: {} | prefabs: {}###layer_{}", layer.Name, layer.CountTiles(), layer.CountColliders(), layer.Prefabs.size(), layer_index);

				if (!ImGui::TreeNode(layer_label.c_str()))
				{
					continue;
				}

				const auto& all_entities = chunk->GetEntities();
				const std::vector<engine::ecs::entity_id_t>* layer_entities = layer_index < all_entities.size() ? &all_entities[layer_index] : nullptr;

				for (size_t i = 0; i < layer.Prefabs.size(); ++i)
				{
					const engine::level::SPrefabInstanceData& instance = layer.Prefabs[i];
					std::string text = std::format("{} ({})", instance.Prefab, instance.Preset.empty() ? "default" : instance.Preset);

					if (layer_entities && i < layer_entities->size())
					{
						const engine::ecs::entity_id_t id = (*layer_entities)[i];
						if (const auto* transform = entities.FindComponent<engine::ecs::components::SCoTransform>(id))
						{
							text += std::format(" @ ({:.0f}, {:.0f}) entity #{}", transform->Position.x, transform->Position.y, id);
						}
					}

					ImGui::PushID(static_cast<int>(i));
					ImGui::BulletText("%s", text.c_str());
					ImGui::SameLine();
					if (ImGui::SmallButton("X"))
					{
						prefab_to_remove = { coord, layer.Name, i };
					}
					ImGui::PopID();
				}

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		if (prefab_to_remove)
		{
			const auto& [coord, layer, index] = *prefab_to_remove;
			level.RemovePrefabInstance(layer, coord, index);
		}

		if (chunk_to_remove)
		{
			level.RemoveChunk(*chunk_to_remove);
			CEditorManager::ShowNotification(notifications::ENotificationType::Info, std::format("Chunk ({}, {}) removed", chunk_to_remove->X, chunk_to_remove->Y));
		}
	}
}
#endif
