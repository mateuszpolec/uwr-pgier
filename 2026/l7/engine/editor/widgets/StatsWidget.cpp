#if defined(DEBUG)
#include <imgui/imgui.h>

#include "StatsWidget.hpp"

#include "engine/game/Game.hpp"
#include "engine/core/FrameTime.hpp"

namespace engine::editor
{
	void CStatsWidget::Render()
	{
    if (ImGui::Begin("Statistics"))
    {
			const engine::core::SFrameTime& m_FrameTime = engine::game::CGame::Get().GetFrameTime();
      const float fps = m_FrameTime.DeltaTime > 0.f ? 1.f / m_FrameTime.DeltaTime : 0.f;

      ImGui::Text("FPS:                %.1f", fps);
      ImGui::Text("Current Frame:      %u", m_FrameTime.CurrentFrame);
      ImGui::Separator();
      ImGui::Text("Delta Time:         %.4f s (%.2f ms)", m_FrameTime.DeltaTime, m_FrameTime.DeltaTime * 1000.f);
      ImGui::Text("Time Elapsed:       %.3f s", m_FrameTime.TimeElapsed);
      ImGui::Separator();
      ImGui::Text("Time Scale:         %.2f", m_FrameTime.TimeScale);
      ImGui::Text("Delta Time Scaled:  %.4f s", m_FrameTime.DeltaTimeScaled);
      ImGui::Text("Time Elapsed Scaled:%.3f s", m_FrameTime.TimeElapsedScaled);
    }
    ImGui::End();
	}
}
#endif
