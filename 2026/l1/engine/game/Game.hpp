#pragma once

#include <SFML/Graphics.hpp>

class CGame
{
    public:

        CGame();
        ~CGame();

        void Run();

    private:

        void Frame();
        void Render();
        void ProcessNativeEvents();

    private:
        sf::Clock m_Clock;
        float m_DeltaTime;

        sf::RenderWindow m_Window;
};