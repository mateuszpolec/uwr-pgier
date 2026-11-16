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
#include "engine/camera/FocusCamera.hpp"

#include "engine/ecs/systems/RenderSystem.hpp"
#include "engine/ecs/systems/AnimationSystem.hpp"

#include "engine/ecs/components/Component_Transform.hpp"
#include "engine/ecs/components/Component_Renderable.hpp"
#include "engine/ecs/components/Component_Animatable.hpp"
#include "engine/ecs/components/Component_Direction.hpp"

namespace engine::game
{
  void CGame::Initialize()
  {
    engine::core::CLogger::Initialize();
    engine::core::InitializeCrashDump();

		m_GameSettings = std::make_unique<engine::game::CGameSettings>();
    m_GameSettings->LoadSettings();

    m_Window.create(sf::VideoMode({ 800, 600 }), "Game Window");

    m_EventDispatcher = std::make_unique<engine::events::CEventDispatcher>(m_FrameTime);

    m_WindowCloseEventHandle = m_EventDispatcher->Subscribe<engine::events::SWindowClosedEvent>([this](const engine::events::SWindowClosedEvent& _event)
      {
        m_Window.close();
      });

    LOG_INFO("Window created with size: {}x{}", m_Window.getSize().x, m_Window.getSize().y);

		// Initialize the job system with 4 worker threads
    m_JobSystem = std::make_unique<engine::core::CJobSystem>(4u);
    LOG_INFO("Job system started with {} threads", m_JobSystem->GetWorkersCount());

    // Load fonts from the assets directory
    m_AssetsManager = std::make_unique<engine::assets::CAssetsManager>();
    m_AssetsManager->Load(engine::filesystem::Assets() / "fonts", true, false);

    m_LevelManager = std::make_unique<engine::level::CLevelManager>();
		m_RenderQueue = std::make_unique<engine::render::CRenderQueue>();

    m_Clock.restart();

#if defined(DEBUG)
    const bool imgui_sfml_init_ok = ImGui::SFML::Init(m_Window);
    ASSERT(imgui_sfml_init_ok, "Failed to initialize ImGui-SFML");

		m_EditorManager = std::make_unique<engine::editor::CEditorManager>();
#endif

    m_GameState = EGameState::LoadingGame;
    LoadGameAssetsAsync();
  }

  void CGame::Shutdown()
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

    if (m_GameState == EGameState::Gameplay)
    {
      m_Player->Frame(m_FrameTime.DeltaTime);

      m_CameraManager->Frame(m_FrameTime.DeltaTime);
      m_LevelManager->Frame(m_CameraManager->GetStreamingWorldRect());
			m_SystemManager->Frame(*m_EntityManager, m_FrameTime.DeltaTime);
		}

#if defined(DEBUG)
    const sf::Time time = sf::seconds(m_FrameTime.DeltaTime);
    ImGui::SFML::Update(m_Window, time);
		m_EditorManager->Frame(m_FrameTime.DeltaTime);
#endif
  }

  void CGame::Render()
  {
    // Clear screen
    m_Window.clear();

    if (m_GameState == EGameState::LoadingGame)
    {
      RenderSplashScreen();
    }

    if (m_GameState == EGameState::Gameplay)
    {
      m_CameraManager->ApplyTo(m_Window);

			m_LevelManager->Render(*m_RenderQueue);
			m_SystemManager->Render(*m_EntityManager, *m_RenderQueue);

			m_RenderQueue->Render(m_Window);
      m_LevelManager->RenderDebug(m_Window);

      // UI is rendered in screen space.
      m_Window.setView(m_Window.getDefaultView());
		}

#if defined(DEBUG)
    m_EditorManager->Render();
    ImGui::SFML::Render(m_Window);
#endif

    // Update the window
    m_Window.display();
  }

  void CGame::ProcessNativeEvents()
  {
    while (const std::optional event = m_Window.pollEvent())
    {
#if defined(DEBUG)
      ImGui::SFML::ProcessEvent(m_Window, *event);

			const bool editor_capture_keyboard = m_EditorManager->WantsCaptureKeyboardInput();
      const bool editor_capture_mouse = m_EditorManager->WantsCaptureMouseInput();
#else
			constexpr bool editor_capture_keyboard = false;
			constexpr bool editor_capture_mouse = false;
#endif

      if (event->is<sf::Event::Closed>())
      {
        m_EventDispatcher->Submit<engine::events::SWindowClosedEvent>();
      }
      else if (const auto* resized = event->getIf<sf::Event::Resized>())
      {
        m_EventDispatcher->Submit<engine::events::SWindowResizedEvent>(resized->size);
      } 
      else if (const auto* focus_lost = event->getIf<sf::Event::FocusLost>())
      {
        m_EventDispatcher->Submit<engine::events::SWindowFocusLostEvent>();
			}
      else if (const auto* key = event->getIf<sf::Event::KeyPressed>())
      {
        const bool is_editor_toggle = key->code == sf::Keyboard::Key::Grave;

        if (editor_capture_keyboard && !is_editor_toggle)
        {
          continue;
        }

        m_EventDispatcher->Submit<engine::events::SKeyPressedEvent>(key->code, key->scancode, key->alt, key->control, key->shift, key->system);
      }
      else if (const auto* key = event->getIf<sf::Event::KeyReleased>())
      {
        if (editor_capture_keyboard)
        {
          continue;
				}

        m_EventDispatcher->Submit<engine::events::SKeyReleasedEvent>(key->code, key->scancode);
      }
      else if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>())
      {
        if (editor_capture_mouse)
        {
          continue;
				}

        m_EventDispatcher->Submit<engine::events::SMouseButtonPressedEvent>(mouse->button, mouse->position);
      }
      else if (const auto* mouse = event->getIf<sf::Event::MouseButtonReleased>())
      {
        if (editor_capture_mouse)
        {
          continue;
        }

        m_EventDispatcher->Submit<engine::events::SMouseButtonReleasedEvent>(mouse->button, mouse->position);
      }
      else if (const auto* mouse = event->getIf<sf::Event::MouseMoved>())
      {
        if (editor_capture_mouse)
        {
          continue;
				}

        m_EventDispatcher->Submit<engine::events::SMouseMovedEvent>(mouse->position);
      }
      else if (const auto* wheel = event->getIf<sf::Event::MouseWheelScrolled>())
      {
        if (editor_capture_mouse)
        {
          continue;
        }

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
        m_AssetsManager->Load(engine::filesystem::Assets(), true, true);
        InitializeEcs();
        InitializeTilesets();
        InitializeAnimations();
      },
      [this]()
      { 
        OnGameAssetsLoaded(); 
      });
  }

  void CGame::OnGameAssetsLoaded()
  {
		LOG_INFO("Game assets loaded successfully, assets count: {}", m_AssetsManager->Size());
		m_GameState = EGameState::LoadingLevel;
    LoadLevel();
  }

  void CGame::LoadLevel()
  {
    InitializePlayer();

    m_CameraManager = std::make_unique<engine::camera::CCameraManager>(m_Window.getSize());
		engine::camera::CFocusCamera& focus_cam = m_CameraManager->Register<engine::camera::CFocusCamera>(m_Player->GetId());
    focus_cam.SnapOnActivate = true;
		m_CameraManager->Push(engine::camera::ECameraType::Focus);

    m_LevelManager->LoadLevel("test");

    m_GameState = EGameState::Gameplay;
  }

  void CGame::InitializeTilesets()
  {
    m_TilesetManager = std::make_unique<engine::assets::CTilesetManager>();
    m_TilesetManager->LoadDirectory(engine::filesystem::Assets() / "tilesets");
		LOG_INFO("Tilesets loaded successfully, tileset count: {}", m_TilesetManager->GetTilesetCount());
  }

  void CGame::InitializeAnimations()
  {
    m_AnimationManager = std::make_unique<engine::animation::CAnimationManager>();
    m_AnimationManager->LoadDirectory(engine::filesystem::Assets() / "animations");
		LOG_INFO("Animations loaded successfully, animation count: {}", m_AnimationManager->GetAnimationCount());
  }

  void CGame::InitializeEcs()
  {
    m_EntityManager = std::make_unique<engine::ecs::CEntityManager>();
    m_SystemManager = std::make_unique<engine::ecs::CSystemManager>();
    m_SystemManager->AddSystem<engine::ecs::systems::CRenderSystem>();
		m_SystemManager->AddSystem<engine::ecs::systems::CAnimationSystem>();

		m_PrefabManager = std::make_unique<engine::ecs::CPrefabManager>();
		m_PrefabManager->RegisterComponent<engine::ecs::components::SCoTransform>();
		m_PrefabManager->RegisterComponent<engine::ecs::components::SCoRenderable>();
		m_PrefabManager->RegisterComponent<engine::ecs::components::SCoAnimatable>();
		m_PrefabManager->RegisterComponent<engine::ecs::components::SCoDirection>();
		m_PrefabManager->LoadDirectory(engine::filesystem::Assets() / "prefabs");
  }

  void CGame::InitializePlayer()
  {
    m_Player = std::make_unique<player::CPlayer>(m_PrefabManager->Instantiate(*m_EntityManager, "player", "big"));
  }
}