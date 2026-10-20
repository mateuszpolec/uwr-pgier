#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>

#if defined(DEBUG)
#include <imgui/SFML/imgui-SFML.h>
#include <imgui/imgui.h>
#endif

#include "Game.hpp"
#include "engine/events/EventDispatcher.hpp"
#include "engine/events/NativeEvents.hpp"
#include "engine/core/Logger.hpp"
#include "engine/core/Assert.hpp"
#include "engine/core/CrashDump.hpp"
#include "engine/filesystem/Paths.hpp"
#include "engine/assets/AssetHandle.hpp"

namespace engine::game
{
  CGame::CGame()
    : m_EventDispatcher(std::make_unique<engine::events::CEventDispatcher>(m_FrameTime))
		, m_AssetsManager(std::make_unique<engine::assets::CAssetsManager>())
		, m_JobSystem(std::make_unique<engine::core::CJobSystem>(4u))
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

		// Load fonts from the assets directory
    m_AssetsManager->Load(engine::filesystem::Assets() / "fonts", true, false);

    LOG_INFO("Job system started with {} threads", m_JobSystem->GetWorkersCount());

    m_Clock.restart();

#if defined(DEBUG)
    const bool imgui_sfml_init_ok = ImGui::SFML::Init(m_Window);
		ASSERT(imgui_sfml_init_ok, "Failed to initialize ImGui-SFML");
#endif

		m_GameState = EGameState::LoadingGame;
    LoadGameAssetsAsync();
  }

  CGame::~CGame()
  {
    m_JobSystem->WaitForAll();

#if defined(DEBUG)
    ImGui::SFML::Shutdown();
#endif

		engine::core::CLogger::Shutdown();
  }

  void CGame::Run()
  {
    while (m_Window.isOpen())
    {
			const float delta_time = m_Clock.restart().asSeconds();
      m_FrameTime.Advance(delta_time);

      Frame();
      Render();
    }
  }

  void CGame::Frame()
  {
    m_JobSystem->Frame();
    ProcessNativeEvents();
    m_EventDispatcher->Frame();

#if defined(DEBUG)
    const sf::Time time = sf::seconds(m_FrameTime.DeltaTime);
    ImGui::SFML::Update(m_Window, time);
    RenderImGuiDebug();
#endif
  }

  void CGame::Render()
  {
    // Clear screen
    m_Window.clear();

#if defined(DEBUG)
    ImGui::SFML::Render(m_Window);
#endif

    if (m_GameState == EGameState::LoadingGame)
    {
      RenderSplashScreen();
    }

    // Update the window
    m_Window.display();
  }

  void CGame::ProcessNativeEvents()
  {
    while (const std::optional event = m_Window.pollEvent())
    {
#if defined(DEBUG)
      ImGui::SFML::ProcessEvent(m_Window, *event);
#endif

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

//////////////// Loading Game ////////////////////////
  void CGame::RenderSplashScreen()
  {
    constexpr float DOT_INTERVAL = 0.5f; // [s]
    constexpr uint32_t MAX_DOTS = 3u;
		constexpr unsigned int FONT_SIZE = 30u;

		engine::assets::TAssetHandle<sf::Font> font_handle = m_AssetsManager->Get<sf::Font>("fonts\\PixelifySans-Medium.ttf");

		const sf::Vector2f window_size = static_cast<sf::Vector2f>(m_Window.getSize());


		sf::Text splash_text(font_handle.Get(), "GAME LOADING", FONT_SIZE);
		splash_text.setFillColor(sf::Color::White);

    // Center text.
    const sf::FloatRect text_bounds = splash_text.getLocalBounds();
    splash_text.setOrigin({ text_bounds.position.x + text_bounds.size.x / 2.f, text_bounds.position.y + text_bounds.size.y / 2.f });
    splash_text.setPosition({ window_size.x / 2.f, window_size.y / 2.f });

    const uint32_t dots_count = static_cast<uint32_t>(m_FrameTime.TimeElapsed / DOT_INTERVAL) % MAX_DOTS + 1u;

    sf::Text dots_text(font_handle.Get(), std::string(dots_count, '.'), FONT_SIZE);
    dots_text.setFillColor(sf::Color::White);
    dots_text.setOrigin({ 0.f, text_bounds.position.y + text_bounds.size.y / 2.f });
    dots_text.setPosition({ splash_text.getPosition().x + text_bounds.size.x / 2.f, window_size.y / 2.f });

    m_Window.draw(splash_text);
    m_Window.draw(dots_text);
  }

  void CGame::LoadGameAssetsAsync()
  {
		LOG_INFO("Loading game assets asynchronously...");

    m_JobSystem->Submit(
      [this]()
      {
				std::this_thread::sleep_for(std::chrono::seconds(5)); // Simulate loading time
        m_AssetsManager->Load(engine::filesystem::Assets(), true, true);
      },
      [this]()
      { 
        OnGameAssetsLoaded(); 
      });
  }

  void CGame::OnGameAssetsLoaded()
  {
		LOG_INFO("Game assets loaded successfully, assets count: {}", m_AssetsManager->Size());
		m_GameState = EGameState::MainMenu;
  }


//////////////// ImGui Debug ////////////////////////
#if defined(DEBUG)
  void CGame::RenderImGuiDebug()
  {
    if (ImGui::Begin("Debug"))
    {
      if (ImGui::CollapsingHeader("Frame Time", ImGuiTreeNodeFlags_DefaultOpen))
      {
        const float fps = m_FrameTime.DeltaTime > 0.f ? 1.f / m_FrameTime.DeltaTime : 0.f;

        ImGui::Text("FPS:                %.1f", fps);
        ImGui::Text("Current Frame:      %u", m_FrameTime.CurrentFrame);
        ImGui::Separator();
        ImGui::Text("Delta Time:         %.4f s (%.2f ms)", m_FrameTime.DeltaTime, m_FrameTime.DeltaTime * 1000.f);
        ImGui::Text("Time Elapsed:       %.3f s", m_FrameTime.TimeElapsed);
        ImGui::Separator();
        ImGui::Text("Time Scale:         %.2f", m_FrameTime.TimeScale);
        ImGui::Text("Delta Time Scaled:  %.4f s", m_FrameTime.DeltaTimeScaled);
        ImGui::Text("Time Elapsed Scaled:%.3f s", m_FrameTime.TimeElapsedScaled);
      }

      if (ImGui::CollapsingHeader("Job System"))
      {
        ImGui::Text("Workers: %u", m_JobSystem->GetWorkersCount());
        ImGui::Text("Idle:    %s", m_JobSystem->IsIdle() ? "yes" : "no");
      }
    }
    ImGui::End();
  }
#endif
}