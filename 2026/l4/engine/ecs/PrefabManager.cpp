#include "PrefabManager.hpp"

#include "engine/core/Logger.hpp"
#include "engine/core/Assert.hpp"
#include "engine/filesystem/JsonSerialization.hpp"

namespace engine::ecs
{
	bool CPrefabManager::LoadPrefab(const std::filesystem::path& _path)
	{
		std::optional<SPrefabDefinition> prefab = engine::filesystem::Load<SPrefabDefinition>(_path);
		if (!prefab)
		{
			LOG_ERROR("Failed to load prefab: {}", _path.string());
			return false;
		}

		const std::string name = _path.stem().string();
		m_Prefabs[name] = std::move(*prefab);
		LOG_INFO("Loaded prefab '{}'", name);
		return true;
	}

	bool CPrefabManager::LoadPresets(const std::string& _prefab, const std::filesystem::path& _path)
	{
		auto presets = engine::filesystem::Load<std::unordered_map<std::string, SPresetDefinition>>(_path);
		if (!presets)
		{
			LOG_ERROR("Failed to load presets: {}", _path.string());
			return false;
		}

		const size_t count = presets->size();
		m_Presets[_prefab].merge(*presets);
		LOG_INFO("Loaded {} presets for prefab '{}'", count, _prefab);
		return true;
	}

	void CPrefabManager::LoadDirectory(const std::filesystem::path& _directory)
	{
		if (!std::filesystem::is_directory(_directory))
		{
			LOG_ERROR("Prefab directory does not exist: {}", _directory.string());
			return;
		}

		for (const auto& entry : std::filesystem::directory_iterator(_directory))
		{
			if (entry.path().extension() != ".json")
			{
				continue;
			}

			if (!entry.path().stem().string().ends_with(PRESETS_SUFFIX))
			{
				LoadPrefab(entry.path());
			}
		}

		for (const auto& entry : std::filesystem::directory_iterator(_directory))
		{
			if (entry.path().extension() != ".json")
			{
				continue;
			}

			const std::string stem = entry.path().stem().string();
			if (!stem.ends_with(PRESETS_SUFFIX))
			{
				continue;
			}

			const std::string prefab = stem.substr(0, stem.size() - PRESETS_SUFFIX.size());
			if (!m_Prefabs.contains(prefab))
			{
				LOG_WARN("Presets file '{}' has no matching prefab '{}'", entry.path().string(), prefab);
				continue;
			}

			LoadPresets(prefab, entry.path());
		}
	}

	bool CPrefabManager::HasPrefab(const std::string& _prefab) const
	{
		return m_Prefabs.contains(_prefab);
	}

	bool CPrefabManager::HasPreset(const std::string& _prefab, const std::string& _preset) const
	{
		const auto it = m_Presets.find(_prefab);
		return it != m_Presets.end() && it->second.contains(_preset);
	}

	CEntity CPrefabManager::Instantiate(CEntityManager& _entity_manager, const std::string& _prefab, const std::string& _preset) const
	{
		const auto prefab_it = m_Prefabs.find(_prefab);
		ASSERT(prefab_it != m_Prefabs.end(), "Unknown prefab");

		std::unordered_map<std::string, nlohmann::json> components = prefab_it->second.Components;

		if (!_preset.empty())
		{
			const SPresetDefinition& preset = FindPreset(_prefab, _preset);

			for (const auto& [name, overrides] : preset.Components)
			{
				const auto base = components.find(name);
				if (base == components.end())
				{
					LOG_WARN("Preset '{}' overrides component '{}' not present in prefab '{}'", _preset, name, _prefab);
					continue;
				}
				base->second.merge_patch(overrides);
			}
		}

		CEntity entity = _entity_manager.CreateEntity();
		ApplyComponents(entity, components);
		return entity;
	}

	void CPrefabManager::ApplyPreset(CEntity _entity, const std::string& _prefab, const std::string& _preset) const
	{
		ApplyComponents(_entity, FindPreset(_prefab, _preset).Components);
	}

	const SPresetDefinition& CPrefabManager::FindPreset(const std::string& _prefab, const std::string& _preset) const
	{
		const auto presets_it = m_Presets.find(_prefab);
		ASSERT(presets_it != m_Presets.end(), "Prefab has no presets");

		const auto preset_it = presets_it->second.find(_preset);
		ASSERT(preset_it != presets_it->second.end(), "Unknown preset");

		return preset_it->second;
	}

	void CPrefabManager::ApplyComponents(CEntity _entity, const std::unordered_map<std::string, nlohmann::json>& _components) const
	{
		for (const auto& [name, json] : _components)
		{
			const auto it = m_ComponentFactories.find(name);
			if (it == m_ComponentFactories.end())
			{
				LOG_ERROR("No component registered under JSON name '{}'", name);
				continue;
			}

			it->second(_entity, json);
		}
	}
}