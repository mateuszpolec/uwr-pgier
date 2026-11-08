#pragma once

#include <filesystem>
#include <unordered_map>
#include <memory>
#include <vector>

#include "Animation.hpp"

namespace engine::animation
{
	class CAnimationManager
	{
		public:

			void LoadDirectory(const std::filesystem::path& _path);
			void LoadAnimation(const std::filesystem::path& _path);

			bool SaveAnimation(const std::string& _name, const SAnimation& _animation);
			bool RemoveAnimation(const std::string& _name);

			[[nodiscard]] const SAnimation* FindAnimation(const std::string& _name) const;
			[[nodiscard]] const SAnimationSequence* FindSequence(const std::string& _name, const std::string& _sequence_name) const;
			[[nodiscard]] const std::unordered_map<std::string, SAnimation>& GetAnimations() const { return m_Animations; }

			[[nodiscard]] size_t GetAnimationCount() const { return m_Animations.size(); }
			[[nodiscard]] bool HasAnimation(const std::string& _name) const;

			[[nodiscard]] static std::filesystem::path GetAnimationFilePath(const std::string& _name);

		private:
			
			std::unordered_map<std::string, SAnimation> m_Animations;
	};
}