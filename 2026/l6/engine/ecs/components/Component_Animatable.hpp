#pragma once

#include <string>
#include <string_view>

#include <nlohmann/json.hpp>

#include "engine/animation/AnimationPlayback.hpp"

namespace engine::ecs::components
{
	struct SCoAnimatable
	{
		static constexpr std::string_view JSON_NAME = "Animatable";
		///////////////////////////////////////////////////////////////////////////////////////
		std::string Animation;
		std::string Sequence;
		float SpeedMultiplier{ 1.f };
		bool AutoPlay{ true };

		engine::animation::CAnimationPlayback Playback;
		std::string ActiveAnimation;
		std::string ActiveSequence;

		void Play(std::string_view _animation, std::string_view _sequence, bool _restart_if_same = false);
		void Play(std::string_view _sequence, bool _restart_if_same = false);

		void Pause() { Playback.Pause(); }
		void Resume() { Playback.Resume(); }
		void Stop() { Playback.Stop(); }

		[[nodiscard]] bool IsPlaying() const { return Playback.IsPlaying(); }
		[[nodiscard]] bool IsFinished() const { return Playback.IsFinished(); }
	};

	void from_json(const nlohmann::json& _j, SCoAnimatable& _component);
}