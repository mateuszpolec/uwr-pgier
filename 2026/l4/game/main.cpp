#include "engine/game/Game.hpp"
#include "engine/filesystem/Paths.hpp"
#include <iostream>

int main()
{
	  engine::game::CGame& game = engine::game::CGame::Get();
    game.Initialize();
    game.Run();
    game.Shutdown();
    return 0;
}