#pragma once

#include <filesystem>
#include <fstream>
#include <optional>

#include <nlohmann/json.hpp>

namespace engine::filesystem
{
	template<typename T>
	bool Save(const std::filesystem::path& _path, const T& _value, int _indent = 4)
	{
		if (!std::filesystem::exists(_path.parent_path()))
		{
			std::filesystem::create_directories(_path.parent_path());
		}

		std::ofstream file(_path);
		if (!file.is_open())
		{
			return false;
		}

		file << nlohmann::json(_value).dump(_indent);
		return file.good();
	}

	template<typename T>
	std::optional<T> Load(const std::filesystem::path& _path)
	{
		if (!std::filesystem::exists(_path))
		{
			return std::nullopt;
		}

		std::ifstream file(_path);

		const auto json = nlohmann::json::parse(file, nullptr, false);

		if (json.is_discarded())
		{
			return std::nullopt;
		}

		return json.get<T>();
	}
}