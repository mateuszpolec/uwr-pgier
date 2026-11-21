#if defined(DEBUG)
#include <algorithm>
#include <imgui/imgui_internal.h>

#include "InfoNotification.hpp"

namespace engine::editor::notifications
{
	CInfoNotification::CInfoNotification(CNotificationsManager& _manager)
		: INotification(_manager)
	{

	}

	void CInfoNotification::Render()
	{
		if (!m_Active)
		{
			return;
		}

		const float alpha = std::clamp(1.f - (m_DisplayTime / MAX_DISPLAY_TIME), MIN_ALPHA, 1.f);

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.15f, 0.15f, 0.15f, alpha));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.2f, 0.36f, 0.67f, alpha));

		ImGui::SetNextWindowPos(m_Position);

		if (ImGui::Begin("Error Notification", &m_Active, DEFAULT_FLAGS))
		{
			ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
			ImGui::TextUnformatted(m_Message.c_str());
		}

		ImGui::End();
		ImGui::PopStyleColor(2);
	}
}
#endif