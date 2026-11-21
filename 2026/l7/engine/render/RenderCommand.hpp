#pragma once

#include <cstdint>
#include <limits>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderStates.hpp>

namespace engine::render
{
	inline constexpr float SORT_Y_BACKGROUND = -std::numeric_limits<float>::infinity();
	inline constexpr float SORT_Y_FOREGROUND = std::numeric_limits<float>::infinity();

	struct SRenderCommand
	{
		int LayerIndex = 0;
		float SortY = 0.f;
		uint32_t Sequence = 0;
		const sf::Drawable* Drawable = nullptr;
		sf::RenderStates States;
	};
}