#pragma once

#include <string>
#include <string_view>
#include <functional>
#include <filesystem>
#include <unordered_map>

#include <nlohmann/json.hpp>

#include "Entity.hpp"
#include "EntityManager.hpp"
#include "engine/core/NonCopyable.hpp"

namespace engine::ecs
{
///////////////////////////////////////////////////////////////////////////////////////
	struct SPrefabDefinition
	{
		std::unordered_map<std::string, nlohmann::json> Components;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SPrefabDefinition, Components)

///////////////////////////////////////////////////////////////////////////////////////
	struct SPresetDefinition
	{
		std::unordered_map<std::string, nlohmann::json> Components;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SPresetDefinition, Components)

///////////////////////////////////////////////////////////////////////////////////////
	class CPrefabManager : public engine::core::NonCopyable
	{
	public:

		using component_factory_t = std::function<void(CEntity, const nlohmann::json&)>;

		template<typename T>
		void RegisterComponent()
		{
			m_ComponentFactories[std::string(T::JSON_NAME)] = [](CEntity _entity, const nlohmann::json& _json)
			{
					T* component = _entity.FindComponent<T>();
					if (!component)
					{
						component = &_entity.AddComponent<T>();
					}

					from_json(_json, *component);
			};
		}

		bool LoadPrefab(const std::filesystem::path& _path);
		bool LoadPresets(const std::string& _prefab, const std::filesystem::path& _path);

		void LoadDirectory(const std::filesystem::path& _directory);

		[[nodiscard]] bool HasPrefab(const std::string& _prefab) const;
		[[nodiscard]] bool HasPreset(const std::string& _prefab, const std::string& _preset) const;

		[[nodiscard]] CEntity Instantiate(CEntityManager& _entity_manager, const std::string& _prefab, const std::string& _preset = {}) const;

		void ApplyPreset(CEntity _entity, const std::string& _prefab, const std::string& _preset) const;

	private:

		[[nodiscard]] const SPresetDefinition& FindPreset(const std::string& _prefab, const std::string& _preset) const;
		void ApplyComponents(CEntity _entity, const std::unordered_map<std::string, nlohmann::json>& _components) const;

	private:

		static constexpr std::string_view PREFAB_EXTENSION = ".prefab";
		static constexpr std::string_view PRESET_EXTENSION = ".presets";

		std::unordered_map<std::string, component_factory_t> m_ComponentFactories;
		std::unordered_map<std::string, SPrefabDefinition> m_Prefabs;
		std::unordered_map<std::string, std::unordered_map<std::string, SPresetDefinition>> m_Presets;
	};
}