#pragma once

namespace engine::game::player
{
///////////////////////////////////////////////////////////////////////////////////////
	enum class EPlayerModule
	{
		Movement,
		Count
	};

	static constexpr inline size_t PlayerModuleCount = static_cast<size_t>(EPlayerModule::Count);
	static constexpr inline size_t PlayerModuleIndex(EPlayerModule _module) 
	{ 
		return static_cast<size_t>(_module);
	}

///////////////////////////////////////////////////////////////////////////////////////
	enum class EPlayerController
	{
		Count
	};

	static constexpr inline size_t PlayerControllerCount = static_cast<size_t>(EPlayerController::Count);
	static constexpr inline size_t PlayerControllerIndex(EPlayerController _controller) 
	{ 
		return static_cast<size_t>(_controller);
	}
}
