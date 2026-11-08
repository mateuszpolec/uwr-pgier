#include "AnimationPlayback.hpp"

#include <algorithm>
#include <cmath>

namespace engine::animation
{
	void CAnimationPlayback::Play(const SAnimationSequence& _sequence, bool _paused)
	{
		m_Direction = _sequence.Speed < 0.f ? -1 : 1;
		m_FrameIndex = m_Direction > 0 ? 0 : static_cast<int>(_sequence.Frames.size()) - 1;
		m_FrameIndex = std::max(m_FrameIndex, 0);
		m_Elapsed = 0.f;
		m_Finished = false;
		m_FrameChanged = true;
		m_Playing = !_paused && !_sequence.Frames.empty();
	}

	void CAnimationPlayback::Stop()
	{
		m_Playing = false;
		m_Elapsed = 0.f;
	}

	void CAnimationPlayback::Finish()
	{
		m_Finished = true;
		m_Playing = false;
		m_Elapsed = 0.f;
	}

	bool CAnimationPlayback::Frame(const SAnimationSequence& _sequence, float _dt, float _speed_multiplier)
	{
		m_FrameChanged = false;

		if (!m_Playing || m_Finished || _sequence.Frames.empty())
		{
			return false;
		}

		const int frame_count = static_cast<int>(_sequence.Frames.size());
		m_FrameIndex = std::clamp(m_FrameIndex, 0, frame_count - 1);

		m_Elapsed += _dt * std::abs(_sequence.Speed) * std::abs(_speed_multiplier);

		while (m_Elapsed >= _sequence.Frames[m_FrameIndex].Duration)
		{
			m_Elapsed -= _sequence.Frames[m_FrameIndex].Duration;

			const int next_frame = m_FrameIndex + m_Direction;
			if (next_frame >= 0 && next_frame < frame_count)
			{
				m_FrameIndex = next_frame;
				m_FrameChanged = true;
				continue;
			}

			switch (_sequence.PlayMode)
			{
			case EPlayMode::Once:
			{
				Finish();
				return true;
			}
			case EPlayMode::Loop:
			{
				m_FrameIndex = m_Direction > 0 ? 0 : frame_count - 1;
				m_FrameChanged = true;
				break;
			}
			case EPlayMode::PingPong:
			{
				m_Direction = -m_Direction;
				m_FrameIndex = std::clamp(m_FrameIndex + m_Direction, 0, frame_count - 1);
				m_FrameChanged = true;
				break;
			}
			case EPlayMode::PingPongOnce:
			{
				const int start_direction = _sequence.Speed < 0.f ? -1 : 1;
				if (m_Direction != start_direction)
				{
					Finish();
					return true;
				}

				m_Direction = -m_Direction;
				m_FrameIndex = std::clamp(m_FrameIndex + m_Direction, 0, frame_count - 1);
				m_FrameChanged = true;
				break;
			}
			}
		}

		return false;
	}
}