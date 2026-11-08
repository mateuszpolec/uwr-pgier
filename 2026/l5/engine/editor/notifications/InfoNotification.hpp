#if defined(DEBUG)
#pragma once

#include "Notification.hpp"

namespace engine::editor::notifications
{
	class CInfoNotification : public INotification
	{
	public:
		CInfoNotification(CNotificationsManager& _manager);
		void Render() override;
	};
}
#endif