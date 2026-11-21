#include "RenderSystem.hpp"

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Transform.hpp>

#include "engine/ecs/components/Component_Transform.hpp"
#include "engine/ecs/components/Component_Renderable.hpp"

namespace engine::ecs::systems
{
	void CRenderSystem::Frame(CEntityManager& _components, float _dt)
	{
		_components.ForEach<components::SCoRenderable>(
			[](entity_id_t, components::SCoRenderable& _renderable)
			{
				_renderable.FindLayer();
			});
	}


	void CRenderSystem::Render(CEntityManager& _components, engine::render::CRenderQueue& _render_queue)
	{
		using namespace engine::ecs::components;

		_components.ForEach<SCoRenderable, SCoTransform>(
			[&_render_queue](entity_id_t, SCoRenderable& _renderable, const SCoTransform& _transform)
			{
				if (const sf::Drawable* drawable = _renderable.GetDrawable())
				{
					if (!drawable)
					{
						return;
					}

					sf::RenderStates states;
					states.transform.translate(_transform.Position);
					states.transform.rotate(_transform.Rotation);
					states.transform.scale(_transform.Scale);

					const sf::FloatRect bounds = states.transform.transformRect(_renderable.GetLocalBounds());
					const float sort_y = bounds.position.y + bounds.size.y;

					_render_queue.Submit(_renderable.LayerIndex, sort_y, *drawable, states);
				}
			});
	}
}
