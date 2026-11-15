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
#include "engine/assets/TilesetManager.hpp"
#include "engine/animation/AnimationsManager.hpp"
#include "engine/level/LevelManager.hpp"
#include "engine/render/RenderQueue.hpp"
#include "engine/camera/CameraManager.hpp"

#if defined(DEBUG)
#include "engine/editor/EditorManager.hpp"
#endif

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

		const sf::RenderWindow& GetWindow() const { return m_Window; }
		sf::RenderWindow& GetWindow() { return m_Window; }

//////////////// Systems getters ////////////////////////
  public:

    [[nodiscard]] const engine::core::SFrameTime& GetFrameTime() const { return m_FrameTime; }
    [[nodiscard]] engine::events::CEventDispatcher& GetEventDispatcher() const { return *m_EventDispatcher; }
    [[nodiscard]] engine::assets::CAssetsManager& GetAssetsManager() const { return *m_AssetsManager; }
		[[nodiscard]] engine::core::CJobSystem& GetJobSystem() const { return *m_JobSystem; }
		[[nodiscard]] engine::ecs::CEntityManager& GetEntityManager() const { return *m_EntityManager; }
    [[nodiscard]] engine::ecs::CSystemManager& GetSystemManager() const { return *m_SystemManager; }
		[[nodiscard]] engine::ecs::CPrefabManager& GetPrefabManager() const { return *m_PrefabManager; }
		[[nodiscard]] engine::game::player::CPlayer& GetPlayer() const { return *m_Player; }
    [[nodiscard]] engine::assets::CTilesetManager& GetTilesetManager() const { return *m_TilesetManager; }
		[[nodiscard]] engine::animation::CAnimationManager& GetAnimationManager() const { return *m_AnimationManager; }
		[[nodiscard]] engine::level::CLevelManager& GetLevelManager() const { return *m_LevelManager; }
		[[nodiscard]] engine::render::CRenderQueue& GetRenderQueue() const { return *m_RenderQueue; }
    [[nodiscard]] engine::camera::CCameraManager& GetCameraManager() const { return *m_CameraManager; }

#if defined(DEBUG)
		[[nodiscard]] engine::editor::CEditorManager& GetEditorManager() const { return *m_EditorManager; }
#endif

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
    void InitializeTilesets();
    void InitializeAnimations();

    void InitializeEcs();
    void InitializePlayer();

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
    std::unique_ptr<engine::assets::CTilesetManager> m_TilesetManager;
    std::unique_ptr<engine::animation::CAnimationManager> m_AnimationManager;
		std::unique_ptr<engine::level::CLevelManager> m_LevelManager;
		std::unique_ptr<engine::render::CRenderQueue> m_RenderQueue;
    std::unique_ptr<engine::camera::CCameraManager> m_CameraManager;

#if defined(DEBUG)
		std::unique_ptr<engine::editor::CEditorManager> m_EditorManager;
#endif
  };
}