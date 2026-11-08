#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <numeric>
#include <filesystem>

#include <nlohmann/json.hpp>

namespace engine::animation
{
///////////////////////////////////////////////////////////////////////////////////////
	enum class EPlayMode
	{
		Once,
		Loop,
		PingPong,
		PingPongOnce
	};

	NLOHMANN_JSON_SERIALIZE_ENUM(EPlayMode, {
		{ EPlayMode::Once, "Once" },
		{ EPlayMode::Loop, "Loop" },
		{ EPlayMode::PingPong, "PingPong" },
		{ EPlayMode::PingPongOnce, "PingPongOnce" },
		})

		inline constexpr std::array<std::pair<EPlayMode, const char*>, 4> PLAY_MODE_NAMES = { {
			{ EPlayMode::Once, "Once" },
			{ EPlayMode::Loop, "Loop" },
			{ EPlayMode::PingPong, "PingPong" },
			{ EPlayMode::PingPongOnce, "PingPongOnce" },
		} };

///////////////////////////////////////////////////////////////////////////////////////
	struct SAnimationFrame
	{
		int TileIndex{ 0 };
		float Duration{ 0.1f };
		std::vector<std::string> Events;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SAnimationFrame, TileIndex, Duration, Events)

///////////////////////////////////////////////////////////////////////////////////////
		struct SAnimationSequence
		{
			std::string Name;
			EPlayMode PlayMode{ EPlayMode::Loop };
			float Speed{ 1.f };
			std::vector<SAnimationFrame> Frames;

			[[nodiscard]] float GetTotalDuration() const
			{
				return std::accumulate(Frames.begin(), Frames.end(), 0.f, [](float sum, const SAnimationFrame& frame) 
				{
					return sum + frame.Duration;
				});
			}
		};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SAnimationSequence, Name, PlayMode, Speed, Frames)

	///////////////////////////////////////////////////////////////////////////////////////
	struct SAnimation
	{
		static constexpr std::string_view EXTENSION = ".anim";
		static bool IsAnimationFile(const std::filesystem::path& _path) { return _path.extension() == EXTENSION; }

		std::string Tileset;
		std::vector<SAnimationSequence> Sequences;

		[[nodiscard]] const SAnimationSequence* FindSequence(const std::string& _name) const
		{
			for (const auto& sequence : Sequences)
			{
				if (sequence.Name == _name)
				{
					return &sequence;
				}
			}
			return nullptr;
		}

		[[nodiscard]] SAnimationSequence* FindSequence(const std::string& _name)
		{
			for (auto& sequence : Sequences)
			{
				if (sequence.Name == _name)
				{
					return &sequence;
				}
			}
			return nullptr;
		}
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SAnimation, Tileset, Sequences)

}