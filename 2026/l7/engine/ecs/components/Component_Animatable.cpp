#include "Component_Animatable.hpp"

namespace engine::ecs::components
{
	void SCoAnimatable::Play(std::string_view _animation, std::string_view _sequence, bool _restart_if_same)
	{
		const bool is_same = _animation == Animation && _sequence == Sequence;
		if (is_same && !_restart_if_same)
		{
			Playback.Resume();
			return;
		}

		Animation = _animation;
		Sequence = _sequence;
		ActiveAnimation.clear();
		ActiveSequence.clear();
		AutoPlay = true;
	}

	void SCoAnimatable::Play(std::string_view _sequence, bool _restart_if_same)
	{
		Play(Animation, _sequence, _restart_if_same);
	}

	void from_json(const nlohmann::json& _j, SCoAnimatable& _component)
	{
		_component.Animation = _j.value("Animation", _component.Animation);
		_component.Sequence = _j.value("Sequence", _component.Sequence);
		_component.SpeedMultiplier = _j.value("Speed", _component.SpeedMultiplier);
		_component.AutoPlay = _j.value("AutoPlay", _component.AutoPlay);

		_component.ActiveSequence.clear();
	}
}