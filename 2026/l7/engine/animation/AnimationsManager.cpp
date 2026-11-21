#include "AnimationsManager.hpp"

#include "engine/core/Logger.hpp"
#include "engine/filesystem/JsonSerialization.hpp"
#include "engine/filesystem/Paths.hpp"

namespace engine::animation
{
	void CAnimationManager::LoadDirectory(const std::filesystem::path& _path)
	{
		if (!std::filesystem::exists(_path) || !std::filesystem::is_directory(_path))
		{
			LOG_ERROR("Animation directory does not exist: {}", _path.string());
			return;
		}

		for (const auto& entry : std::filesystem::directory_iterator(_path))
		{
			if (SAnimation::IsAnimationFile(_path))
			{
				continue;
			}

			LoadAnimation(entry.path());
		}
	}

	void CAnimationManager::LoadAnimation(const std::filesystem::path& _path)
	{
		std::optional<SAnimation> animation = engine::filesystem::Load<SAnimation>(_path);
		if (!animation)
		{
			LOG_ERROR("Failed to load animation: {}", _path.string());
			return;
		}

		const std::string name = _path.stem().string();
		m_Animations.insert_or_assign(name, std::move(*animation));
		LOG_INFO("Loaded animation '{}'", name);
	}

	bool CAnimationManager::SaveAnimation(const std::string& _name, const SAnimation& _animation)
	{
		if (!engine::filesystem::Save(GetAnimationFilePath(_name), _animation))
		{
			LOG_ERROR("Failed to save animation '{}'", _name);
			return false;
		}

		m_Animations.insert_or_assign(_name, _animation);
		return true;
	}

	bool CAnimationManager::RemoveAnimation(const std::string& _name)
	{
		std::error_code ec;
		std::filesystem::remove(GetAnimationFilePath(_name), ec);
		return m_Animations.erase(_name) > 0;
	}

	const SAnimation* CAnimationManager::FindAnimation(const std::string& _name) const
	{
		return m_Animations.find(_name) != m_Animations.end() ? &m_Animations.at(_name) : nullptr;
	}

	const SAnimationSequence* CAnimationManager::FindSequence(const std::string& _name, const std::string& _sequence_name) const
	{
		const SAnimation* animation = FindAnimation(_name);
		return animation ? animation->FindSequence(_sequence_name) : nullptr;
	}

	bool CAnimationManager::HasAnimation(const std::string& _name) const
	{
		return m_Animations.find(_name) != m_Animations.end();
	}

	std::filesystem::path CAnimationManager::GetAnimationFilePath(const std::string& _name)
	{
		std::filesystem::path path = engine::filesystem::Assets() / "animations" / _name;
		path += SAnimation::EXTENSION;
		return path;
	}
}