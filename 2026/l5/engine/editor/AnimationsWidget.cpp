#if defined(DEBUG)
#include "AnimationsWidget.hpp"

#include <algorithm>

#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <imgui/SFML/imgui-SFML.h>
#include <SFML/Graphics/Sprite.hpp>

#include "EditorManager.hpp"
#include "notifications/NotificationType.hpp"
#include "engine/game/Game.hpp"
#include "engine/assets/TilesetManager.hpp"
#include "engine/animation/AnimationsManager.hpp"

namespace engine::editor
{
	///////////////////////////////////////////////////////////////////////////////////////
	void CAnimationsWidget::Frame(float _dt)
	{
		if (m_Visible)
		{
			if (const auto* sequence = GetSelectedSequence())
			{
				m_Preview.Frame(*sequence, _dt);
			}
		}
	}

	void CAnimationsWidget::Render()
	{
		if (ImGui::Begin(GetName().data(), &m_Visible, ImGuiWindowFlags_MenuBar))
		{
			RenderMenuBar();

			ImGui::BeginChild("Animations", { 180.f, 0.f }, ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
			RenderAnimationList();
			ImGui::EndChild();

			ImGui::SameLine();

			ImGui::BeginChild("Sequences", { 180.f, 0.f }, ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
			if (m_SelectedAnimation.empty())
			{
				ImGui::TextDisabled("Select an animation.");
			}
			else
			{
				RenderSequenceList();
			}
			ImGui::EndChild();

			ImGui::SameLine();

			ImGui::BeginChild("Editor", { 0.f, 0.f }, ImGuiChildFlags_Borders);
			if (!GetSelectedSequence())
			{
				ImGui::TextDisabled("Select a sequence.");
			}
			else
			{
				RenderSequenceEditor();
			}
			ImGui::EndChild();

			RenderCreateAnimationPopup();
		}

		ImGui::End();
	}

	///////////////////////////////////////////////////////////////////////////////////////
	void CAnimationsWidget::RenderMenuBar()
	{
		if (!ImGui::BeginMenuBar())
		{
			return;
		}

		auto& animations = engine::game::CGame::Get().GetAnimationManager();

		if (ImGui::MenuItem("New animation"))
		{
			m_NewAnimationName.clear();
			m_OpenCreatePopup = true;
		}

		ImGui::BeginDisabled(m_SelectedAnimation.empty() || !m_Dirty);
		if (ImGui::MenuItem("Save"))
		{
			if (animations.SaveAnimation(m_SelectedAnimation, m_Draft))
			{
				m_Dirty = false;
				CEditorManager::ShowNotification(notifications::ENotificationType::Success, "Animation saved");
			}
			else
			{
				CEditorManager::ShowNotification(notifications::ENotificationType::Error, "Failed to save animation");
			}
		}
		ImGui::EndDisabled();

		ImGui::BeginDisabled(m_SelectedAnimation.empty());
		if (ImGui::MenuItem("Delete animation"))
		{
			animations.RemoveAnimation(m_SelectedAnimation);
			m_SelectedAnimation.clear();
			m_SelectedSequence = -1;
			m_Dirty = false;
		}
		ImGui::EndDisabled();

		ImGui::EndMenuBar();
	}

	///////////////////////////////////////////////////////////////////////////////////////
	void CAnimationsWidget::RenderAnimationList()
	{
		for (const auto& [name, animation] : engine::game::CGame::Get().GetAnimationManager().GetAnimations())
		{
			const bool selected = name == m_SelectedAnimation;
			const std::string label = selected && m_Dirty ? name + " *" : name;

			if (ImGui::Selectable(label.c_str(), selected))
			{
				SelectAnimation(name);
			}
		}
	}

	void CAnimationsWidget::RenderSequenceList()
	{
		auto& tilesets = engine::game::CGame::Get().GetTilesetManager();

		ImGui::SetNextItemWidth(-1.f);
		if (ImGui::BeginCombo("##Tileset", m_Draft.Tileset.empty() ? "<tileset>" : m_Draft.Tileset.c_str()))
		{
			for (const auto& [name, tileset] : tilesets.GetTilesets())
			{
				if (ImGui::Selectable(name.c_str(), name == m_Draft.Tileset))
				{
					m_Draft.Tileset = name;
					m_Dirty = true;
					RestartPreview();
				}
			}
			ImGui::EndCombo();
		}

		ImGui::Separator();

		for (int i = 0; i < static_cast<int>(m_Draft.Sequences.size()); ++i)
		{
			ImGui::PushID(i);
			if (ImGui::Selectable(m_Draft.Sequences[i].Name.c_str(), i == m_SelectedSequence))
			{
				SelectSequence(i);
			}

			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::MenuItem("Duplicate"))
				{
					engine::animation::SAnimationSequence copy = m_Draft.Sequences[i];
					copy.Name += "_copy";
					m_Draft.Sequences.insert(m_Draft.Sequences.begin() + i + 1, std::move(copy));
					m_Dirty = true;
				}
				if (ImGui::MenuItem("Remove"))
				{
					m_Draft.Sequences.erase(m_Draft.Sequences.begin() + i);
					m_SelectedSequence = -1;
					m_Dirty = true;
					ImGui::EndPopup();
					ImGui::PopID();
					break;
				}
				ImGui::EndPopup();
			}
			ImGui::PopID();
		}

		ImGui::Separator();

		ImGui::SetNextItemWidth(-60.f);
		ImGui::InputTextWithHint("##NewSeq", "sequence name", &m_NewSequenceName);
		ImGui::SameLine();

		const bool exists = m_Draft.FindSequence(m_NewSequenceName) != nullptr;
		ImGui::BeginDisabled(m_NewSequenceName.empty() || exists);
		if (ImGui::Button("Add"))
		{
			engine::animation::SAnimationSequence sequence;
			sequence.Name = m_NewSequenceName;
			sequence.Frames.push_back({});
			m_Draft.Sequences.push_back(std::move(sequence));
			m_NewSequenceName.clear();
			m_Dirty = true;
			SelectSequence(static_cast<int>(m_Draft.Sequences.size()) - 1);
		}
		ImGui::EndDisabled();
	}

	///////////////////////////////////////////////////////////////////////////////////////
	void CAnimationsWidget::SelectAnimation(const std::string& _name)
	{
		const auto* animation = engine::game::CGame::Get().GetAnimationManager().FindAnimation(_name);
		if (!animation)
		{
			return;
		}

		m_SelectedAnimation = _name;
		m_Draft = *animation;
		m_Dirty = false;
		m_PickerTarget = -1;
		SelectSequence(m_Draft.Sequences.empty() ? -1 : 0);
	}

	void CAnimationsWidget::SelectSequence(int _index)
	{
		m_SelectedSequence = _index;
		m_PickerTarget = -1;
		RestartPreview();
	}

	void CAnimationsWidget::RestartPreview()
	{
		if (const auto* sequence = GetSelectedSequence())
		{
			m_Preview.Play(*sequence);
		}
		else
		{
			m_Preview.Stop();
		}
	}

	engine::animation::SAnimationSequence* CAnimationsWidget::GetSelectedSequence()
	{
		if (m_SelectedSequence < 0 || m_SelectedSequence >= static_cast<int>(m_Draft.Sequences.size()))
		{
			return nullptr;
		}
		return &m_Draft.Sequences[m_SelectedSequence];
	}

	const engine::assets::STileset* CAnimationsWidget::GetTileset() const
	{
		const auto* tileset = engine::game::CGame::Get().GetTilesetManager().FindTileset(m_Draft.Tileset);
		return tileset && tileset->IsValid() ? tileset : nullptr;
	}

	///////////////////////////////////////////////////////////////////////////////////////
	void CAnimationsWidget::RenderSequenceEditor()
	{
		auto& sequence = *GetSelectedSequence();

		if (ImGui::InputText("Name", &sequence.Name))
		{
			m_Dirty = true;
		}

		const char* current_mode = "?";
		for (const auto& [mode, label] : engine::animation::PLAY_MODE_NAMES)
		{
			if (mode == sequence.PlayMode)
			{
				current_mode = label;
			}
		}

		if (ImGui::BeginCombo("Play mode", current_mode))
		{
			for (const auto& [mode, label] : engine::animation::PLAY_MODE_NAMES)
			{
				if (ImGui::Selectable(label, mode == sequence.PlayMode))
				{
					sequence.PlayMode = mode;
					m_Dirty = true;
					RestartPreview();
				}
			}
			ImGui::EndCombo();
		}

		if (ImGui::DragFloat("Speed", &sequence.Speed, 0.05f, -10.f, 10.f, "%.2f"))
		{
			m_Dirty = true;
		}

		ImGui::Text("Total: %.2fs, %zu frames", sequence.GetTotalDuration(), sequence.Frames.size());

		ImGui::Separator();
		RenderPreview();
		ImGui::Separator();
		RenderFramesTable();

		if (m_PickerTarget >= 0)
		{
			ImGui::Separator();
			RenderTilePicker();
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////
	void CAnimationsWidget::RenderPreview()
	{
		const auto* tileset = GetTileset();
		auto& sequence = *GetSelectedSequence();

		if (!tileset || sequence.Frames.empty())
		{
			ImGui::TextDisabled("No preview (missing tileset or frames)");
			return;
		}

		const int frame_index = std::clamp(m_Preview.GetFrameIndex(), 0, static_cast<int>(sequence.Frames.size()) - 1);

		if (const auto rect = tileset->GetTileRect(sequence.Frames[frame_index].TileIndex))
		{
			sf::Sprite sprite(tileset->Texture.Get());
			sprite.setTextureRect(*rect);
			ImGui::Image(sprite, { rect->size.x * m_PreviewScale, rect->size.y * m_PreviewScale });
		}

		ImGui::SameLine();
		ImGui::BeginGroup();
		ImGui::Text("Frame %d / %zu", frame_index, sequence.Frames.size());
		ImGui::TextUnformatted(m_Preview.IsFinished() ? "Finished" : (m_Preview.IsPlaying() ? "Playing" : "Paused"));

		if (ImGui::Button(m_Preview.IsPlaying() ? "Pause" : "Play"))
		{
			if (m_Preview.IsPlaying())
			{
				m_Preview.Pause();
			}
			else if (m_Preview.IsFinished())
			{
				RestartPreview();
			}
			else
			{
				m_Preview.Resume();
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Restart"))
		{
			RestartPreview();
		}

		ImGui::SetNextItemWidth(100.f);
		ImGui::SliderFloat("Zoom", &m_PreviewScale, 1.f, 8.f, "%.0fx");
		ImGui::EndGroup();
	}

	///////////////////////////////////////////////////////////////////////////////////////
	void CAnimationsWidget::RenderFramesTable()
	{
		const auto* tileset = GetTileset();
		auto& sequence = *GetSelectedSequence();

		if (ImGui::Button("Add frame"))
		{
			engine::animation::SAnimationFrame frame;
			if (!sequence.Frames.empty())
			{
				frame.TileIndex = sequence.Frames.back().TileIndex + 1;
				frame.Duration = sequence.Frames.back().Duration;
			}
			sequence.Frames.push_back(std::move(frame));
			m_Dirty = true;
			RestartPreview();
		}

		constexpr ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit;
		if (!ImGui::BeginTable("Frames", 6, flags))
		{
			return;
		}

		ImGui::TableSetupColumn("#");
		ImGui::TableSetupColumn("Tile");
		ImGui::TableSetupColumn("Index");
		ImGui::TableSetupColumn("Duration");
		ImGui::TableSetupColumn("Events", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("");
		ImGui::TableHeadersRow();

		int to_remove = -1;
		int to_move_from = -1;
		int to_move_to = -1;

		for (int i = 0; i < static_cast<int>(sequence.Frames.size()); ++i)
		{
			auto& frame = sequence.Frames[i];
			ImGui::PushID(i);
			ImGui::TableNextRow();

			ImGui::TableNextColumn();
			if (i == m_Preview.GetFrameIndex())
			{
				ImGui::TextColored({ 1.f, 0.8f, 0.2f, 1.f }, "> %d", i);
			}
			else
			{
				ImGui::Text("%d", i);
			}

			ImGui::TableNextColumn();
			if (tileset)
			{
				if (const auto rect = tileset->GetTileRect(frame.TileIndex))
				{
					sf::Sprite sprite(tileset->Texture.Get());
					sprite.setTextureRect(*rect);
					if (ImGui::ImageButton("pick", sprite, { 32.f, 32.f }))
					{
						m_PickerTarget = i;
					}
				}
			}

			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(60.f);
			if (ImGui::InputInt("##tile", &frame.TileIndex, 0))
			{
				m_Dirty = true;
			}

			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(70.f);
			if (ImGui::DragFloat("##dur", &frame.Duration, 0.01f, 0.01f, 10.f, "%.2fs"))
			{
				m_Dirty = true;
			}

			ImGui::TableNextColumn();
			for (int e = 0; e < static_cast<int>(frame.Events.size()); ++e)
			{
				ImGui::PushID(e);
				ImGui::SetNextItemWidth(-30.f);
				if (ImGui::InputText("##ev", &frame.Events[e]))
				{
					m_Dirty = true;
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("x"))
				{
					frame.Events.erase(frame.Events.begin() + e);
					m_Dirty = true;
					ImGui::PopID();
					break;
				}
				ImGui::PopID();
			}
			if (ImGui::SmallButton("+ event"))
			{
				frame.Events.emplace_back();
				m_Dirty = true;
			}

			ImGui::TableNextColumn();
			if (ImGui::ArrowButton("up", ImGuiDir_Up) && i > 0)
			{
				to_move_from = i;
				to_move_to = i - 1;
			}
			ImGui::SameLine();
			if (ImGui::ArrowButton("down", ImGuiDir_Down) && i + 1 < static_cast<int>(sequence.Frames.size()))
			{
				to_move_from = i;
				to_move_to = i + 1;
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("Remove"))
			{
				to_remove = i;
			}

			ImGui::PopID();
		}

		ImGui::EndTable();

		if (to_move_from >= 0)
		{
			std::swap(sequence.Frames[to_move_from], sequence.Frames[to_move_to]);
			m_Dirty = true;
			RestartPreview();
		}

		if (to_remove >= 0)
		{
			sequence.Frames.erase(sequence.Frames.begin() + to_remove);
			m_Dirty = true;
			m_PickerTarget = -1;
			RestartPreview();
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////
	void CAnimationsWidget::RenderTilePicker()
	{
		const auto* tileset = GetTileset();
		auto& sequence = *GetSelectedSequence();

		if (!tileset || m_PickerTarget >= static_cast<int>(sequence.Frames.size()))
		{
			m_PickerTarget = -1;
			return;
		}

		ImGui::Text("Pick tile for frame %d", m_PickerTarget);
		ImGui::SameLine();
		if (ImGui::SmallButton("Close"))
		{
			m_PickerTarget = -1;
			return;
		}

		ImGui::BeginChild("Picker", { 0.f, 220.f }, ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);

		constexpr float TILE_PX = 32.f;
		const int current = sequence.Frames[m_PickerTarget].TileIndex;

		for (int i = 0; i < tileset->GetTileCount(); ++i)
		{
			if (i % tileset->Columns != 0)
			{
				ImGui::SameLine(0.f, 2.f);
			}

			const auto rect = tileset->GetTileRect(i);
			if (!rect)
			{
				continue;
			}

			sf::Sprite sprite(tileset->Texture.Get());
			sprite.setTextureRect(*rect);

			ImGui::PushID(i);
			ImGui::PushStyleColor(ImGuiCol_Button, i == current ? ImVec4(1.f, 0.8f, 0.2f, 0.6f) : ImVec4(0, 0, 0, 0));
			if (ImGui::ImageButton("tile", sprite, { TILE_PX, TILE_PX }))
			{
				sequence.Frames[m_PickerTarget].TileIndex = i;
				m_Dirty = true;
			}
			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Tile %d", i);
			}
			ImGui::PopID();
		}

		ImGui::EndChild();
	}

	///////////////////////////////////////////////////////////////////////////////////////
	void CAnimationsWidget::RenderCreateAnimationPopup()
	{
		if (m_OpenCreatePopup)
		{
			ImGui::OpenPopup("New Animation");
			m_OpenCreatePopup = false;
		}

		if (!ImGui::BeginPopupModal("New Animation", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			return;
		}

		auto& animations = engine::game::CGame::Get().GetAnimationManager();

		ImGui::InputText("Name", &m_NewAnimationName);

		const bool exists = animations.HasAnimation(m_NewAnimationName);
		if (exists)
		{
			ImGui::TextColored({ 1.f, 0.4f, 0.4f, 1.f }, "Animation with this name already exists");
		}

		ImGui::BeginDisabled(m_NewAnimationName.empty() || exists);
		if (ImGui::Button("Create"))
		{
			if (animations.SaveAnimation(m_NewAnimationName, engine::animation::SAnimation{}))
			{
				SelectAnimation(m_NewAnimationName);
				CEditorManager::ShowNotification(notifications::ENotificationType::Success, "Animation created");
			}
			else
			{
				CEditorManager::ShowNotification(notifications::ENotificationType::Error, "Failed to create animation");
			}

			ImGui::CloseCurrentPopup();
		}
		ImGui::EndDisabled();

		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}
#endif