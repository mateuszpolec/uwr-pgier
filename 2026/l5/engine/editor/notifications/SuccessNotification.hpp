#if defined(DEBUG)

#pragma once

#include "Notification.hpp"

namespace engine::editor::notifications
{
	class CSuccessNotification : public INotification
	{
	public:
		CSuccessNotification(CNotificationsManager& _manager);
		void Render() override;
	};
}
#endif