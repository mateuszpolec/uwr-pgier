#pragma once

#include "Animation.hpp"

namespace engine::animation
{
	class CAnimationPlayback
	{
	public:

		void Play(const SAnimationSequence& _sequence, bool _paused = false);
		void Pause() { m_Playing = false; }
		void Resume() { m_Playing = !m_Finished; }
		void Stop();

		bool Frame(const SAnimationSequence& _sequence, float _dt, float _speed_multiplier = 1.f);

		[[nodiscard]] int GetFrameIndex() const { return m_FrameIndex; }
		[[nodiscard]] bool IsPlaying() const { return m_Playing; }
		[[nodiscard]] bool IsFinished() const { return m_Finished; }
		[[nodiscard]] bool HasFrameChanged() const { return m_FrameChanged; }

	private:

		void Finish();

	private:

		int m_FrameIndex{ 0 };
		int m_Direction{ 1 };
		float m_Elapsed{ 0.f };
		bool m_Playing{ false };
		bool m_Finished{ false };
		bool m_FrameChanged{ false };
	};
}