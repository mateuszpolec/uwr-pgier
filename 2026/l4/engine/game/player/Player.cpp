#include <SFML/Graphics/RectangleShape.hpp>

#include "Player.hpp"
#include "PlayerMovementModule.hpp"

#include "engine/ecs/components/Component_Transform.hpp"
#include "engine/ecs/components/Component_Renderable.hpp"

namespace engine::game::player 
{
	CPlayer::CPlayer(engine::ecs::CEntity _entity)
		: CEntity(_entity)
	{
		InitializeModules();
	}

	void CPlayer::Frame(float _dt)
	{
		for (auto& module : m_Modules)
		{
			module->Frame(_dt);
		}
	}

	void game::player::CPlayer::InitializeModules()
	{
		m_Modules[static_cast<size_t>(EPlayerModule::Movement)] = std::make_unique<CPlayerMovementModule>(*this);
	}
}
