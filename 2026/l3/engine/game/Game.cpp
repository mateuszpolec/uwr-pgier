#include <imgui/SFML/imgui-SFML.h>

#include "Game.hpp"
#include "engine/events/EventDispatcher.hpp"
#include "engine/events/NativeEvents.hpp"
#include "engine/core/Logger.hpp"
#include "engine/core/Assert.hpp"
#include "engine/core/CrashDump.hpp"

namespace engine::game
{
  CGame::CGame()
    : m_EventDispatcher(std::make_unique<engine::events::CEventDispatcher>(m_FrameTime))
  {

		engine::core::CLogger::Initialize();
    engine::core::InitializeCrashDump();

    m_Window.create(sf::VideoMode({ 800, 600 }), "Game Window");
    m_Window.setFramerateLimit(60);

    m_WindowCloseEventHandle = m_EventDispatcher->Subscribe<engine::events::SWindowClosedEvent>([this](const engine::events::SWindowClosedEvent& _event)
    {
      m_Window.close();
    });

		LOG_INFO("Window created with size: {}x{}", m_Window.getSize().x, m_Window.getSize().y);

    m_Clock.restart();

#if defined(DEBUG)
    const bool imgui_sfml_init_ok = ImGui::SFML::Init(m_Window);
		ASSERT(imgui_sfml_init_ok, "Failed to initialize ImGui-SFML");
#endif
  }

  CGame::~CGame()
  {
#if defined(DEBUG)
    ImGui::SFML::Shutdown();
#endif

		engine::core::CLogger::Shutdown();
  }

  void CGame::Run()
  {
    while (m_Window.isOpen())
    {
			const float delta_time = m_Clock.getElapsedTime().asSeconds();
      m_FrameTime.Advance(delta_time);

      Frame();
      Render();
    }
  }

  void CGame::Frame()
  {
    ProcessNativeEvents();
    m_EventDispatcher->Frame();

#if defined(DEBUG)
    const sf::Time time = sf::seconds(m_FrameTime.DeltaTime);
    ImGui::SFML::Update(m_Window, time);
#endif
  }

  void CGame::Render()
  {
#if defined(DEBUG)
    ImGui::SFML::Render(m_Window);
#endif

    // Clear screen
    m_Window.clear();

    // Update the window
    m_Window.display();
  }

  void CGame::ProcessNativeEvents()
  {
    while (const std::optional event = m_Window.pollEvent())
    {
      ImGui::SFML::ProcessEvent(m_Window, *event);

      if (event->is<sf::Event::Closed>())
      {
        m_EventDispatcher->Submit<engine::events::SWindowClosedEvent>();
      }
      else if (const auto* resized = event->getIf<sf::Event::Resized>())
      {
        m_EventDispatcher->Submit<engine::events::SWindowResizedEvent>(resized->size);
      }
      else if (const auto* key = event->getIf<sf::Event::KeyPressed>())
      {
        m_EventDispatcher->Submit<engine::events::SKeyPressedEvent>(
          key->code, key->scancode, key->alt, key->control, key->shift, key->system);
      }
      else if (const auto* key = event->getIf<sf::Event::KeyReleased>())
      {
        m_EventDispatcher->Submit<engine::events::SKeyReleasedEvent>(key->code, key->scancode);
      }
      else if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>())
      {
        m_EventDispatcher->Submit<engine::events::SMouseButtonPressedEvent>(mouse->button, mouse->position);
      }
      else if (const auto* mouse = event->getIf<sf::Event::MouseButtonReleased>())
      {
        m_EventDispatcher->Submit<engine::events::SMouseButtonReleasedEvent>(mouse->button, mouse->position);
      }
      else if (const auto* mouse = event->getIf<sf::Event::MouseMoved>())
      {
        m_EventDispatcher->Submit<engine::events::SMouseMovedEvent>(mouse->position);
      }
      else if (const auto* wheel = event->getIf<sf::Event::MouseWheelScrolled>())
      {
        m_EventDispatcher->Submit<engine::events::SMouseWheelScrolledEvent>(wheel->delta, wheel->position);
      }
    }
  }
}