#pragma once

#include <variant>
#include <string>
#include <unordered_map>
#include <filesystem>

#include "engine/core/Hash.hpp"
#include "engine/core/Assert.hpp"

namespace engine::game
{
	//////////////////////////////////////////////////////////////////////////
	class CGameVariables
	{
		public:

			static constexpr std::string_view EXTENSION = ".vars";
			using value_t = std::variant<int, float, std::string, bool>;

		public:

			static bool Load();

			template<typename T>
			[[nodiscard]] static T Get(const std::string& _var_name)
			{
				const uint64_t hash = engine::core::Fnv1a64(_var_name);
				auto it = m_Variables.find(hash);
				ASSERT(it != m_Variables.end(), "Variable not found: " + _var_name);

				if (auto value = std::get_if<T>(&it->second))
				{
					return *value;
				}

				ASSERT(false, "Variable type mismatch: " + _var_name);
				return T{};
			}

			template<typename T>
			[[nodiscard]] T& GetRef(const std::string& _var_name)
			{
				const uint64_t hash = engine::core::Fnv1a64(_var_name);
				auto it = m_Variables.find(hash);

				ASSERT(it != m_Variables.end(), "Variable not found: " + _var_name);
				if (auto value = std::get_if<T>(&it->second))
				{
					return *value;
				}

				ASSERT(false, "Variable type mismatch: " + _var_name);
				throw std::runtime_error("Variable type mismatch: " + _var_name);
			}

			[[nodiscard]] static std::filesystem::path GetVariablesFilePath();

		private:

			static std::unordered_map<uint64_t, value_t> m_Variables;

	};
}