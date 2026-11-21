#pragma once

#include <cstdint>
#include <functional>
#include <vector>

#include <SFML/Graphics/RenderTarget.hpp>

#include "engine/core/NonCopyable.hpp"
#include "RenderCommand.hpp"

namespace engine::render
{
	///////////////////////////////////////////////////////////////////////////////////////
	struct SRenderStats
	{
		size_t Submitted = 0;
		size_t Drawn = 0;
		size_t Culled = 0;
	};

	///////////////////////////////////////////////////////////////////////////////////////
	class CRenderQueue : public engine::core::NonCopyable
	{
	public:

		CRenderQueue() = default;
		~CRenderQueue() = default;

		void Submit(int _layer_index, float _sort_y, const sf::Drawable& _drawable, const sf::RenderStates& _states = sf::RenderStates::Default);
		void Submit(SRenderCommand _command);

		void Render(sf::RenderTarget& _target);
		void Clear();

		[[nodiscard]] size_t Size() const { return m_Commands.size(); }
		[[nodiscard]] const SRenderStats& GetLastStats() const { return m_LastStats; }

	private:

		std::vector<SRenderCommand> m_Commands;
		uint32_t m_Sequence = 0;
		SRenderStats m_LastStats;
	};
}