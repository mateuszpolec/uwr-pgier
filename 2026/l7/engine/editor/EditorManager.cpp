#if defined(DEBUG)
#include <imgui/imgui.h>
#include <SFML/Window/Keyboard.hpp>

#include "EditorManager.hpp"
#include "engine/events/EventDispatcher.hpp"
#include "engine/events/NativeEvents.hpp"
#include "engine/game/Game.hpp"
#include "engine/game/GameVariables.hpp"

#include "engine/editor/widgets/StatsWidget.hpp"
#include "engine/editor/widgets/JobSystemWidget.hpp"
#include "engine/editor/widgets/AssetsBrowserWidget.hpp"
#include "engine/editor/widgets/AnimationsWidget.hpp"
#include "engine/editor/widgets/SceneViewWidget.hpp"
#include "engine/editor/widgets/LevelEditorWidget.hpp"
#include "engine/editor/widgets/CameraWidget.hpp"

namespace engine::editor
{
	std::unique_ptr<notifications::CNotificationsManager> CEditorManager::s_NotificationsManager = std::make_unique<notifications::CNotificationsManager>();

	CEditorManager::CEditorManager()
	{
		m_Widgets.emplace_back(std::make_unique<CStatsWidget>());
		m_Widgets.emplace_back(std::make_unique<CJobSystemWidget>());
		m_Widgets.emplace_back(std::make_unique<CAssetsBrowserWidget>());
		m_Widgets.emplace_back(std::make_unique<CAnimationsWidget>());
		m_Widgets.emplace_back(std::make_unique<CSceneViewWidget>());
		m_Widgets.emplace_back(std::make_unique<CLevelEditorWidget>());
		m_Widgets.emplace_back(std::make_unique<CCameraWidget>());

		m_ToggleStateEventHandle = engine::game::CGame::Get().GetEventDispatcher().Subscribe<engine::events::SKeyPressedEvent>(
			[this](const engine::events::SKeyPressedEvent& _event)
			{
				if (_event.Code == sf::Keyboard::Key::Grave)
				{
					ToggleState();
				}
			});
	}
	void CEditorManager::Frame(float _dt)
	{
		if (m_State == EEditorState::Hidden)
		{
			return;
		}

		s_NotificationsManager->Frame(_dt);

		for (auto& widget : m_Widgets)
		{
			if (widget->IsVisible())
			{
				widget->Frame(_dt);
			}
		}
	}

	void CEditorManager::Render()
	{
		if (m_State == EEditorState::Hidden)
		{
			return;
		}

		RenderMenuBar();

		s_NotificationsManager->Render();

		for (auto& widget : m_Widgets)
		{
			if (widget->IsVisible())
			{
				widget->Render();
			}
		}
	}

	void CEditorManager::ShowNotification(notifications::ENotificationType _type, std::string _message)
	{
		s_NotificationsManager->ShowNotification(_type, _message);
	}

	void CEditorManager::ToggleState()
	{
		switch (m_State)
		{
			case EEditorState::Active:
			{
				SetState(EEditorState::Transparent);
				break;
			}
			case EEditorState::Transparent:
			{
				SetState(EEditorState::Hidden);
				break;
			}
			case EEditorState::Hidden:
			{
				SetState(EEditorState::Active);
				break;
			}
		}
	}

	void CEditorManager::SetState(EEditorState _state)
	{
		m_State = _state;
		ApplyState();
	}

	void CEditorManager::ApplyState()
	{
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();

		switch (m_State)
		{
		case EEditorState::Active:
		{
			io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
			io.ConfigFlags &= ~ImGuiConfigFlags_NoKeyboard;
			style.Alpha = 1.0f;
			break;
		}
		case EEditorState::Transparent:
		{
			io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
			io.ConfigFlags |= ImGuiConfigFlags_NoKeyboard;
			style.Alpha = 0.35f;
			break;
		}
		case EEditorState::Hidden:
		{
			io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
			io.ConfigFlags |= ImGuiConfigFlags_NoKeyboard;
			style.Alpha = 1.0f;
			break;
		}
		}
	}

	bool CEditorManager::WantsCaptureMouseInput() const
	{
		if (m_State == EEditorState::Active)
		{
			return true;
		}
		else if (m_State == EEditorState::Transparent)
		{
			return ImGui::GetIO().WantCaptureMouse;
		}

		return false;
	}

	bool CEditorManager::WantsCaptureKeyboardInput() const
	{
		if (m_State == EEditorState::Active)
		{
			return true;
		}
		else if (m_State == EEditorState::Transparent)
		{
			return ImGui::GetIO().WantCaptureKeyboard;
		}

		return false;
	}

	void CEditorManager::RenderMenuBar()
	{
		if (ImGui::BeginMainMenuBar())
		{
			std::map<std::string_view, std::vector<IWidget*>> menu_widgets;

			for (auto& widget : m_Widgets)
			{
				if (widget->GetRenderInMenu())
				{
					menu_widgets[std::string_view(widget->GetMenuName())].push_back(widget.get());
				}
			}

			for (const auto& [menu_name, widgets] : menu_widgets)
			{
				if (ImGui::BeginMenu(menu_name.data()))
				{
					for (auto* widget : widgets)
					{
						bool visible = widget->IsVisible();
						if (ImGui::MenuItem(widget->GetName().data(), nullptr, &visible))
						{
							widget->SetVisible(visible);
						}
					}
					ImGui::EndMenu();
				}
			}

			if (ImGui::BeginMenu("Help"))
			{
				ImGui::TextDisabled("Press '~' to toggle ImGui state");
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Reload"))
			{
				if (ImGui::MenuItem("Reload Game Variables"))
				{
					engine::game::CGameVariables::Load();
				}
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}
}
#endif