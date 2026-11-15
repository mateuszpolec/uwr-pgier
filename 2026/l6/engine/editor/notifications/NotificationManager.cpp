
#if defined(DEBUG)

#include <algorithm>

#include "NotificationManager.hpp"

#include "InfoNotification.hpp"
#include "WarningNotification.hpp"
#include "SuccessNotification.hpp"
#include "ErrorNotification.hpp"

namespace engine::editor::notifications
{

	void CNotificationsManager::Frame(float _dt)
	{
		for (auto it = m_Notifications.begin(); it != m_Notifications.end();)
		{
			auto& notification = *it;

			notification->Frame(_dt);

			if (notification->IsExpired())
			{
				notification->Hide();
				it = m_Notifications.erase(it);
				SortNotifications();
				continue;
			}

			it++;
		}
	}

	void CNotificationsManager::Render()
	{
		for (std::unique_ptr<INotification>& notification : m_Notifications)
		{
			notification->Render();
		}
	}

	void CNotificationsManager::ShowNotification(ENotificationType _type, std::string _message)
	{
		m_Notifications.emplace_back(CreateNotification(_type));
		m_Notifications.back()->Show(static_cast<int>(m_Notifications.size()), _message);
		SortNotifications();
	}

	void CNotificationsManager::SortNotifications()
	{
		std::sort(m_Notifications.begin(), m_Notifications.end(), [](const auto& _lhs, const auto& _rhs)
			{
				return _lhs->GetDisplayTime() > _rhs->GetDisplayTime();
			});
	}

	std::unique_ptr<INotification> CNotificationsManager::CreateNotification(ENotificationType _type)
	{
		switch (_type)
		{
		case ENotificationType::Info: return std::make_unique<CInfoNotification>(*this);
		case ENotificationType::Success: return std::make_unique<CSuccessNotification>(*this);
		case ENotificationType::Warning: return std::make_unique<CWarningNotification>(*this);
		case ENotificationType::Error: return std::make_unique<CErrorNotification>(*this);
		}

		return nullptr;
	}
}
#endif