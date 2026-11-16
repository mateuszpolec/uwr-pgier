#include "GameSettings.hpp"
#include "engine/filesystem/JsonSerialization.hpp"
#include "engine/core/Logger.hpp"
#include "engine/filesystem/Paths.hpp"

namespace engine::game
{
		void CGameSettings::SetStreamingRadius(int radius)
	{
		m_StreamingRadius = radius;
		SaveSettings();
	}
	int CGameSettings::GetStreamingRadius() const
	{
		return m_StreamingRadius;
	}

	void CGameSettings::SetWindowResolution(int width, int height)
	{
		m_WindowResolution = { width, height };
		SaveSettings();
	}
	sf::Vector2i CGameSettings::GetWindowResolution() const
	{
		return m_WindowResolution;
	}

	void CGameSettings::SaveSettings()
	{
		const std::filesystem::path path = GetSettingsFilePath();
		if (!engine::filesystem::Save(path, *this))
		{
			LOG_ERROR("Failed to save game settings to '{}'", path.string());
		}
		else
		{
			LOG_INFO("Game settings saved to '{}'", path.string());
		}
	}

	void CGameSettings::LoadSettings()
	{
		const std::filesystem::path path = GetSettingsFilePath();
		std::optional<CGameSettings> settings = engine::filesystem::Load<CGameSettings>(path);
		if (settings)
		{
			m_StreamingRadius = settings->m_StreamingRadius;
			m_WindowResolution = settings->m_WindowResolution;
			LOG_INFO("Game settings loaded from '{}'", path.string());
		}
		else
		{
			LOG_WARN("No game settings found at '{}', using defaults", path.string());
			m_StreamingRadius = 1;
			m_WindowResolution = { 800, 600 };
			SaveSettings();
		}
	}

	std::filesystem::path CGameSettings::GetSettingsFilePath()
	{
		const std::filesystem::path settings_dir = engine::filesystem::AppData() / "settings";
		return settings_dir / "game_settings.json";
	}
}