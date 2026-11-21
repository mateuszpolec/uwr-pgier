#pragma once

namespace engine::game::player
{
	class CPlayer;

	class IPlayerModule
	{
		public:

			IPlayerModule(CPlayer& _player)
				: m_Player(_player)
			{
			}

			virtual ~IPlayerModule() = default;

			virtual void Frame(float _dt) = 0;


		protected:

			CPlayer& m_Player;
	};
}