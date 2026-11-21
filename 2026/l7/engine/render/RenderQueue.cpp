#include <algorithm>

#include "RenderQueue.hpp"
#include "engine/game/Game.hpp"
#include "engine/level/LevelManager.hpp"

namespace engine::render
{
	void CRenderQueue::Submit(int _layer_index, float _sort_y, const sf::Drawable& _drawable, const sf::RenderStates& _states)
	{
		SRenderCommand command;
		command.LayerIndex = _layer_index;
		command.SortY = _sort_y;
		command.Drawable = &_drawable;
		command.States = _states;
		Submit(std::move(command));
	}

	void CRenderQueue::Submit(SRenderCommand _command)
	{
		if (!_command.Drawable)
		{
			return;
		}

		_command.Sequence = m_Sequence++;
		m_Commands.push_back(std::move(_command));
	}

	void CRenderQueue::Render(sf::RenderTarget& _target)
	{
		m_LastStats = {};
		m_LastStats.Submitted = m_Commands.size();

		const engine::level::CLevelManager& level_manager = engine::game::CGame::Get().GetLevelManager();

		std::sort(m_Commands.begin(), m_Commands.end(), [](const SRenderCommand& _a, const SRenderCommand& _b)
			{
				if (_a.LayerIndex != _b.LayerIndex)
				{
					return _a.LayerIndex < _b.LayerIndex;
				}
				if (_a.SortY != _b.SortY)
				{
					return _a.SortY < _b.SortY;
				}
				return _a.Sequence < _b.Sequence;
			});

		for (const SRenderCommand& command : m_Commands)
		{
			if (!level_manager.IsLayerVisible(command.LayerIndex))
			{
				++m_LastStats.Culled;
				continue;
			}

			_target.draw(*command.Drawable, command.States);
			++m_LastStats.Drawn;
		}

		Clear();
	}

	void CRenderQueue::Clear()
	{
		m_Commands.clear();
		m_Sequence = 0;
	}
}