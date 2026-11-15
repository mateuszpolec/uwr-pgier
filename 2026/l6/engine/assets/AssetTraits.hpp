#pragma once

#include <array>
#include <string_view>

#include <SFML/System/InputStream.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

#include "AssetTypes.hpp"


namespace engine::assets
{
	///////////////////////////////////////////////////////////////////////////////////////
	template<typename T>
	struct TAssetTraits;

	///////////////////////////////////////////////////////////////////////////////////////
	template<>
	struct TAssetTraits<sf::Texture>
	{
		static constexpr EAssetType TYPE = EAssetType::Texture;
		static constexpr std::array<std::string_view, 4> EXTENSIONS = { ".png", ".jpg", ".jpeg", ".bmp" };
		static constexpr bool CACHE_STREAM = false;

		[[nodiscard]] static bool Load(sf::Texture& _texture, sf::InputStream& _stream)
		{
			return _texture.loadFromStream(_stream);
		}
	};

	///////////////////////////////////////////////////////////////////////////////////////
	template<>
	struct TAssetTraits<sf::Font>
	{
		static constexpr EAssetType TYPE = EAssetType::Font;
		static constexpr std::array<std::string_view, 2> EXTENSIONS = { ".ttf", ".otf" };
		static constexpr bool CACHE_STREAM = true;

		[[nodiscard]] static bool Load(sf::Font& _font, sf::InputStream& _stream)
		{
			return _font.openFromStream(_stream);
		}
	};

	///////////////////////////////////////////////////////////////////////////////////////
	template<>
	struct TAssetTraits<sf::SoundBuffer>
	{
		static constexpr EAssetType TYPE = EAssetType::Sound;
		static constexpr std::array<std::string_view, 3> EXTENSIONS = { ".wav", ".ogg", ".flac" };
		static constexpr bool CACHE_STREAM = false;

		[[nodiscard]] static bool Load(sf::SoundBuffer& _sound_buffer, sf::InputStream& _stream)
		{
			return _sound_buffer.loadFromStream(_stream);
		}
	};
}