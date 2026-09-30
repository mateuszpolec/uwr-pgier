#pragma once

#include <SFML/Graphics.hpp>

class CGame
{
    public:

        CGame() = default;
        ~CGame() = default;

        void Initialize();
        void Run();
        void Shutdown();

    private:

        void Frame();
        void Render();
        void ProcessNativeEvents();

    private:
        sf::Clock m_Clock;
        float m_DeltaTime;

        sf::RenderWindow m_Window;
};