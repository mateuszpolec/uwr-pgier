#include <imgui/SFML/imgui-SFML.h>

#include "Game.hpp"

CGame::CGame()
{
  m_Window.create(sf::VideoMode({ 800, 600 }), "Game Window");
  m_DeltaTime = 0.0f;
  m_Clock.restart();
  m_Window.setFramerateLimit(60);

#if defined(DEBUG)
  const bool imgui_sfml_init_ok = ImGui::SFML::Init(m_Window);

  if (!imgui_sfml_init_ok)
  {
    throw std::runtime_error("Failed to initialize ImGui-SFML");
  }
#endif
}

CGame::~CGame()
{
#if defined(DEBUG)
  ImGui::SFML::Shutdown();
#endif
}

void CGame::Run()
{
    while (m_Window.isOpen())
    {
        m_DeltaTime = m_Clock.restart().asSeconds();

        Frame();
        Render();
    }
}

void CGame::Frame()
{
    ProcessNativeEvents();

#if defined(DEBUG)
		const sf::Time time = sf::seconds(m_DeltaTime);
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
    while (const std::optional<sf::Event> event = m_Window.pollEvent())
    {
      if (event->is<sf::Event::Closed>())
      {
				m_Window.close();
      }
    }
}