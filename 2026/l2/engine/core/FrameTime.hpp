#pragma once

#include <cstdint>

namespace engine::core
{
	struct SFrameTime
	{
		float DeltaTime = 0.f;
		float TimeElapsed = 0.f;
		float TimeScale = 1.f;
		float DeltaTimeScaled = 0.f;
		float TimeElapsedScaled = 0.f;
		uint32_t CurrentFrame = 0;

		void Advance(float _dt)
		{
			DeltaTime = _dt;
			TimeElapsed += DeltaTime;
			DeltaTimeScaled = DeltaTime * TimeScale;
			TimeElapsedScaled += DeltaTimeScaled;
			CurrentFrame++;
		}
	};
}