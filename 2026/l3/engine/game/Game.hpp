#pragma once

#include <SFML/Graphics.hpp>
#include "engine/events/EventHandle.hpp"
#include "engine/core/FrameTime.hpp"
#include "engine/assets/AssetManager.hpp"
#include "engine/core/JobSystem.hpp"

namespace engine::events
{
	class CEventDispatcher;
}


namespace engine::game
{
  enum class EGameState
  {
		Initializing, // Initialize engine subsystems
		LoadingGame, // Show splash screen & load assets
		MainMenu,
		LoadingLevel, // Show loading screen & load level assets
		Gameplay
  };

  class CGame
  {
  public:

    CGame();
    ~CGame();

    void Run();

    [[nodiscard]] const engine::core::SFrameTime& GetFrameTime() const { return m_FrameTime; }

  private:

    void Frame();
    void Render();
    void ProcessNativeEvents();

//////////////// Loading Game ////////////////////////
  private:
    void RenderSplashScreen();
		void LoadGameAssetsAsync();
		void OnGameAssetsLoaded();

//////////////// ImGui Debug ////////////////////////
#if defined(DEBUG)
  private:
		void RenderImGuiDebug();
#endif

  private:

    sf::Clock m_Clock;
    sf::RenderWindow m_Window;

		engine::events::CEventHandle m_WindowCloseEventHandle;
    engine::core::SFrameTime m_FrameTime;
		EGameState m_GameState = EGameState::Initializing;

    std::unique_ptr<engine::events::CEventDispatcher> m_EventDispatcher;
		std::unique_ptr<engine::assets::CAssetsManager> m_AssetsManager;
		std::unique_ptr<engine::core::CJobSystem> m_JobSystem;
  };
}