#pragma once

#include <filesystem>
#include <nlohmann/json.hpp>
#include <SFML/System/Vector2.hpp>

#include "engine/filesystem/JsonSerializationSfml.hpp"

namespace engine::game
{
	class CGameSettings
	{
	public:

		void SetStreamingRadius(int radius);
		int GetStreamingRadius() const;

		void SetWindowResolution(int width, int height);
		sf::Vector2i GetWindowResolution() const;

		void LoadSettings();

	private:

		void SaveSettings();

		[[nodiscard]] std::filesystem::path GetSettingsFilePath();

	private:

		sf::Vector2i m_WindowResolution{ 800, 600 };
		int m_StreamingRadius = 1;

		NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(CGameSettings, m_StreamingRadius, m_WindowResolution)
	};
}