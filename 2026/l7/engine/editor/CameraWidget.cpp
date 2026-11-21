#if defined(DEBUG)
#include <format>

#include <imgui/imgui.h>

#include "CameraWidget.hpp"
#include "engine/camera/CameraManager.hpp"
#include "engine/camera/FocusCamera.hpp"
#include "engine/camera/FreeCamera.hpp"
#include "engine/game/Game.hpp"
#include "engine/level/LevelManager.hpp"
#include "engine/game/GameSettings.hpp"

namespace engine::editor
{
	void CCameraWidget::Render()
	{
		if (!ImGui::Begin("Camera", &m_Visible))
		{
			ImGui::End();
			return;
		}

		engine::camera::CCameraManager& cameras = engine::game::CGame::Get().GetCameraManager();
		if (!cameras.HasActive())
		{
			ImGui::TextDisabled("No active camera.");
			ImGui::End();
			return;
		}

		RenderStack();
		ImGui::Separator();
		RenderActiveState();
		ImGui::Separator();

		if (ImGui::CollapsingHeader("Focus camera", ImGuiTreeNodeFlags_DefaultOpen))
		{
			RenderFocusCameraSettings();
		}
		if (ImGui::CollapsingHeader("Free camera"))
		{
			RenderFreeCameraSettings();
		}
		if (ImGui::CollapsingHeader("Streaming"))
		{
			RenderStreamingInfo();
		}

		ImGui::End();
	}

	void CCameraWidget::RenderStack()
	{
		using namespace engine::camera;
		CCameraManager& cameras = engine::game::CGame::Get().GetCameraManager();

		ImGui::TextUnformatted("Stack (bottom -> top):");
		const auto& stack = cameras.GetStack();
		for (size_t i = 0; i < stack.size(); ++i)
		{
			const bool top = i + 1 == stack.size();
			ImGui::BulletText("%s%s", ToString(stack[i]).data(), top ? "  (active)" : "");
		}

		if (ImGui::Button("Pop top"))
		{
			cameras.Pop();
		}
		ImGui::SameLine();
		ImGui::BeginDisabled(stack.size() <= 1);
		ImGui::EndDisabled();
	}

	void CCameraWidget::RenderActiveState()
	{
		using namespace engine::camera;
		CCameraManager& cameras = engine::game::CGame::Get().GetCameraManager();
		SCameraState& state = cameras.GetActive().GetState();

		ImGui::Text("Active: %s", ToString(cameras.GetActiveType()).data());

		float center[2] = { state.Center.x, state.Center.y };
		if (ImGui::DragFloat2("Center", center, 1.f))
		{
			state.Center = { center[0], center[1] };
		}

		ImGui::SliderFloat("Zoom", &state.Zoom, 0.1f, 8.f, "%.3f", ImGuiSliderFlags_Logarithmic);
		ImGui::SameLine();
		if (ImGui::SmallButton("1:1"))
		{
			state.Zoom = 1.f;
		}

		const sf::Vector2f velocity = cameras.GetVelocity();
		ImGui::Text("Velocity: %.1f, %.1f px/s", velocity.x, velocity.y);

		const sf::FloatRect visible = cameras.GetVisibleWorldRect();
		ImGui::Text("Visible: [%.0f, %.0f] %.0f x %.0f", visible.position.x, visible.position.y, visible.size.x, visible.size.y);
	}

	void CCameraWidget::RenderFocusCameraSettings()
	{
		using namespace engine::camera;
		CFocusCamera* camera = engine::game::CGame::Get().GetCameraManager().Get<CFocusCamera>(ECameraType::Focus);
		if (!camera)
		{
			ImGui::TextDisabled("Not registered.");
			return;
		}

		ImGui::PushID("Focus");
		if (camera->GetTarget())
		{
			ImGui::Text("Target entity: %llu", static_cast<unsigned long long>(*camera->GetTarget()));
		}
		else
		{
			ImGui::TextDisabled("No target.");
		}

		ImGui::DragFloat("Smoothing", &camera->Smoothing, 0.1f, 0.f, 50.f);

		float offset[2] = { camera->Offset.x, camera->Offset.y };
		if (ImGui::DragFloat2("Offset", offset, 1.f))
		{
			camera->Offset = { offset[0], offset[1] };
		}

		ImGui::Checkbox("Snap on activate", &camera->SnapOnActivate);
		if (ImGui::Button("Snap to target"))
		{
			camera->SnapToTarget();
		}
		ImGui::PopID();
	}

	void CCameraWidget::RenderFreeCameraSettings()
	{
		using namespace engine::camera;
		CFreeCamera* camera = engine::game::CGame::Get().GetCameraManager().Get<CFreeCamera>(ECameraType::Free);
		if (!camera)
		{
			ImGui::TextDisabled("Not registered.");
			return;
		}

		const bool is_active = engine::game::CGame::Get().GetCameraManager().IsActive(ECameraType::Free);

		if (is_active)
		{
			if (ImGui::Button("Disable free camera"))
			{
				engine::game::CGame::Get().GetCameraManager().Pop();
			}
		}
		else
		{
			if (ImGui::Button("Enable free camera"))
			{
				engine::game::CGame::Get().GetCameraManager().Push(ECameraType::Free);
			}
		}
	}

	void CCameraWidget::RenderStreamingInfo()
	{
		auto& game = engine::game::CGame::Get();
		engine::camera::CCameraManager& cameras = game.GetCameraManager();
		engine::level::CLevelManager& level = game.GetLevelManager();
		engine::game::CGameSettings& settings = game.GetGameSettings();

		ImGui::SliderFloat("Lookahead [s]", &cameras.StreamingLookahead, 0.f, 3.f);

		const sf::FloatRect streaming = cameras.GetStreamingWorldRect();
		ImGui::Text("Streaming rect: [%.0f, %.0f] %.0f x %.0f", streaming.position.x, streaming.position.y, streaming.size.x, streaming.size.y);

		if (level.HasLevel())
		{
			ImGui::Text("Chunks loaded: %zu, pending: %zu", level.GetChunks().size(), level.GetPendingLoadsCount());
			ImGui::Text("Streaming radius: %d chunk(s)", settings.GetStreamingRadius());
		}
		else
		{
			ImGui::TextDisabled("No level loaded.");
		}
	}
}
#endif