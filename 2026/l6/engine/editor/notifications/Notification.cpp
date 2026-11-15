#if defined(DEBUG)

#include "Notification.hpp"
#include "NotificationManager.hpp"

namespace engine::editor::notifications
{

	INotification::INotification(CNotificationsManager& _manager)
		: m_Manager(_manager)
		, m_DisplayTime(0.f)
		, m_Active(true)
		, m_Position(DEFAULT_POSITION)
	{

	}

	void INotification::Frame(float dt)
	{
		m_DisplayTime += dt;
	}


	bool INotification::IsExpired() const
	{
		return m_DisplayTime >= MAX_DISPLAY_TIME;
	}

	bool INotification::IsActive() const
	{
		return m_Active;
	}

	float INotification::GetDisplayTime() const
	{
		return m_DisplayTime;
	}

	void INotification::Show(int _active_notifications, std::string _message)
	{
		m_Position = CalculatePosition(_active_notifications);
		m_Message = std::move(_message);
	}

	void INotification::Hide()
	{
		m_DisplayTime = 0.f;
		m_Active = false;
		m_Message.clear();
	}

	ImVec2 INotification::CalculatePosition(int _active_notifications) const
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();

		constexpr float PADDING = 30.f;
		ImVec2 position;
		position.x = viewport->WorkPos.x + viewport->WorkSize.x - DEFAULT_SIZE.x - PADDING;
		position.y = viewport->WorkPos.y + PADDING;

		if (_active_notifications > 1)
		{
			position.y += (_active_notifications - 1) * (DEFAULT_SIZE.y + PADDING);
		}

		return position;
	}
}
#endif