#include "GameVariables.hpp"

#include "engine/filesystem/JsonSerialization.hpp"
#include "engine/filesystem/Paths.hpp"
#include "engine/core/Logger.hpp"

namespace engine::game
{
	std::unordered_map<uint64_t, CGameVariables::value_t> CGameVariables::m_Variables;

	bool CGameVariables::Load()
	{
		const std::optional<nlohmann::json> json = engine::filesystem::Load<nlohmann::json>(GetVariablesFilePath());

		if (!json || !json->is_object())
		{
			LOG_ERROR("Failed to load game variables from file: {}", GetVariablesFilePath().string());
			return false;
		}

		m_Variables.clear();
		m_Variables.reserve(json->size());

		for (const auto& [key, value] : json->items())
		{
			const uint64_t hash = engine::core::Fnv1a64(key);
			if (value.is_number_integer())
			{
				m_Variables[hash] = value.get<int>();
			}
			else if (value.is_number_float())
			{
				m_Variables[hash] = value.get<float>();
			}
			else if (value.is_string())
			{
				m_Variables[hash] = value.get<std::string>();
			}
			else if (value.is_boolean())
			{
				m_Variables[hash] = value.get<bool>();
			}
			else
			{
				LOG_WARN("Unsupported variable type for '{}'", key);
			}
		}

		LOG_INFO("Loaded {} game variables from file: {}", m_Variables.size(), GetVariablesFilePath().string());
		return true;
	}

	std::filesystem::path CGameVariables::GetVariablesFilePath()
	{
		std::filesystem::path path = engine::filesystem::Assets() / "scripts" / "game_vars";
		path += CGameVariables::EXTENSION;
		return path;
	}
}