#include "Game.hpp"

void CGame::Initialize()
{
  m_Window.create(sf::VideoMode({ 800, 600 }), "Game Window");
    m_DeltaTime = 0.0f;
    m_Clock.restart();
    m_Window.setFramerateLimit(60);
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
}
    
void CGame::Render()
{
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
            m_Window.close();
    }
}