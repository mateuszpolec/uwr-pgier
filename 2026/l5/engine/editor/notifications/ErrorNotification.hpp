#if defined(DEBUG)
#pragma once

#include "Notification.hpp"
#include "NotificationManager.hpp"

namespace engine::editor::notifications
{
	class CErrorNotification : public INotification
	{
	public:
		CErrorNotification(CNotificationsManager& _manager);
		void Render() override;
	};
}
#endif