#if defined(DEBUG)
#include <imgui/imgui.h>

#include "JobSystemWidget.hpp"

#include "engine/game/Game.hpp"
#include "engine/core/FrameTime.hpp"

namespace engine::editor
{
  void CJobSystemWidget::Render()
  {
    if (ImGui::Begin("Job System"))
    {
      ImGui::Text("Workers: %u", engine::game::CGame::Get().GetJobSystem().GetWorkersCount());
      ImGui::Text("Idle:    %s", engine::game::CGame::Get().GetJobSystem().IsIdle() ? "yes" : "no");
    }
    ImGui::End();
  }
}
#endif
