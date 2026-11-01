#pragma once

#include <SFML/Graphics.hpp>
#include "engine/events/EventHandle.hpp"
#include "engine/assets/AssetManager.hpp"
#include "engine/core/JobSystem.hpp"
#include "engine/core/Singleton.hpp"
#include "engine/core/FrameTime.hpp"
#include "engine/ecs/EntityManager.hpp"
#include "engine/ecs/SystemManager.hpp"
#include "engine/ecs/PrefabManager.hpp"
#include "engine/game/player/Player.hpp"
#include "engine/events/EventDispatcher.hpp"

namespace engine::game
{
///////////////////////////////////////////////////////////////////////////////////////
  enum class EGameState
  {
		Initializing, // Initialize engine subsystems
		LoadingGame, // Show splash screen & load assets
		MainMenu,
		LoadingLevel, // Show loading screen & load level assets
		Gameplay
  };

///////////////////////////////////////////////////////////////////////////////////////
	class CGame : public engine::core::Singleton<CGame>
  {
  public:

    CGame() = default;
    ~CGame() = default;

    void Initialize();
    void Shutdown();

    void Run();

    [[nodiscard]] const engine::core::SFrameTime& GetFrameTime() const { return m_FrameTime; }

//////////////// Systems getters ////////////////////////
  public:

    [[nodiscard]] engine::events::CEventDispatcher& GetEventDispatcher() const { return *m_EventDispatcher; }
    [[nodiscard]] engine::assets::CAssetsManager& GetAssetsManager() const { return *m_AssetsManager; }
		[[nodiscard]] engine::core::CJobSystem& GetJobSystem() const { return *m_JobSystem; }
		[[nodiscard]] engine::ecs::CEntityManager& GetEntityManager() const { return *m_EntityManager; }
    [[nodiscard]] engine::ecs::CSystemManager& GetSystemManager() const { return *m_SystemManager; }
		[[nodiscard]] engine::ecs::CPrefabManager& GetPrefabManager() const { return *m_PrefabManager; }
		[[nodiscard]] engine::game::player::CPlayer& GetPlayer() const { return *m_Player; }

  private:

    void Frame();
    void Render();
    void ProcessNativeEvents();

//////////////// Loading Game ////////////////////////
  private:
    void RenderSplashScreen();
		void LoadGameAssetsAsync();
		void OnGameAssetsLoaded();

//////////////// Loading Level ////////////////////////
    void LoadLevel();

    void InitializeEcs();
    void InitializePlayer();

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
		std::unique_ptr<engine::ecs::CEntityManager> m_EntityManager;
		std::unique_ptr<engine::ecs::CSystemManager> m_SystemManager;
		std::unique_ptr<engine::ecs::CPrefabManager> m_PrefabManager;
		std::unique_ptr<engine::game::player::CPlayer> m_Player;
  };
}