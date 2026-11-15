#if defined(DEBUG)

#pragma once

#include <vector>
#include <memory>
#include <string>

#include "Notification.hpp"
#include "NotificationType.hpp"

namespace engine::editor::notifications
{
	///////////////////////////////////////////////////////////////////////////////////////////////////
	class CNotificationsManager
	{
	public:

		CNotificationsManager() = default;
		~CNotificationsManager() = default;

		void Frame(float _dt);
		void Render();

		void ShowNotification(ENotificationType _type, std::string _message);

	private:

		std::unique_ptr<INotification> CreateNotification(ENotificationType _type);
		void SortNotifications();

	private:

		std::vector<std::unique_ptr<INotification>> m_Notifications;

	};
}
#endif