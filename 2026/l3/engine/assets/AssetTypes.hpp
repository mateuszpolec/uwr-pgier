#pragma once

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

namespace engine::assets
{
	enum class EAssetType
	{
		Unknown = 0,
		Texture,
		Font,
		Sound
	};

	enum class EAssetLoadState
	{
		Unloaded = 0,
		Pending,
		Loaded,
		Failed
	};
}