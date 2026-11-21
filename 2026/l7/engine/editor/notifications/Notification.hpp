
#if defined(DEBUG)

#pragma once

#include <string>
#include <imgui/imgui.h>

namespace engine::editor::notifications
{
	class CNotificationsManager;

	class INotification
	{
	public:

		INotification(CNotificationsManager& _manager);
		virtual ~INotification() = default;

		void Frame(float dt);
		virtual void Render() = 0;

		[[nodiscard]] bool IsExpired() const;
		[[nodiscard]] bool IsActive() const;
		[[nodiscard]] float GetDisplayTime() const;

		void Show(int _active_notifications, std::string _message);
		void Hide();

	private:

		ImVec2 CalculatePosition(int _active_notifications) const;

	protected:

		constexpr static float MAX_DISPLAY_TIME = 8.f;
		constexpr static float MIN_ALPHA = 0.5f;
		constexpr static ImVec2 DEFAULT_POSITION{ 5.f, 42.f };
		constexpr static ImVec2 DEFAULT_SIZE{ 250.f, 42.f };
		static constexpr ImGuiWindowFlags DEFAULT_FLAGS =
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_NoNav |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoInputs;

	protected:

		float m_DisplayTime;
		ImVec2 m_Position;

		bool m_Active;
		std::string m_Message;

		CNotificationsManager& m_Manager;
	};
}
#endif