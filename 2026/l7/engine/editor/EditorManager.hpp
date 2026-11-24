#if defined(DEBUG)
#pragma once

#include <vector>
#include <memory>

#include "engine/events/EventHandle.hpp"
#include "notifications/NotificationManager.hpp"
#include "engine/editor/widgets/Widget.hpp"

namespace engine::editor
{
///////////////////////////////////////////////////////////////////////////
	enum class EEditorState
	{
		Active,
		Transparent,
		Hidden
	};

///////////////////////////////////////////////////////////////////////////
	class CEditorManager
	{
		public:

			CEditorManager();
			~CEditorManager() = default;

			void Frame(float _dt);
			void Render();

			static void ShowNotification(notifications::ENotificationType _type, std::string _message);

			void ToggleState();
			void SetState(EEditorState _state);
			void ApplyState();

			bool WantsCaptureMouseInput() const;
			bool WantsCaptureKeyboardInput() const;

		private:

			void RenderMenuBar();

		private:

			EEditorState m_State = EEditorState::Hidden;
			engine::events::CEventHandle m_ToggleStateEventHandle;

			std::vector<std::unique_ptr<IWidget>> m_Widgets;
			static std::unique_ptr<notifications::CNotificationsManager> s_NotificationsManager;

	};
}
#endif