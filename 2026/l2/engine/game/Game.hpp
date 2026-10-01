#pragma once

#include <SFML/Graphics.hpp>
#include "engine/events/EventHandle.hpp"
#include "engine/core/FrameTime.hpp"

namespace engine::events
{
	class CEventDispatcher;
}

namespace engine::game
{
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

  private:

    sf::Clock m_Clock;

		engine::events::CEventHandle m_WindowCloseEventHandle;

    engine::core::SFrameTime m_FrameTime;

    std::unique_ptr<engine::events::CEventDispatcher> m_EventDispatcher;

    sf::RenderWindow m_Window;
  };
}