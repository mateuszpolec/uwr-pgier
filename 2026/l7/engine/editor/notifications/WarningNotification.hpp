#if defined(DEBUG)

#pragma once

#include "Notification.hpp"

namespace engine::editor::notifications
{
	class CWarningNotification : public INotification
	{
	public:
		CWarningNotification(CNotificationsManager& _manager);
		void Render() override;
	};
}
#endif