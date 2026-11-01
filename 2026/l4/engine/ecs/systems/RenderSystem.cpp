#include "RenderSystem.hpp"

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Transform.hpp>

#include "engine/ecs/components/Component_Transform.hpp"
#include "engine/ecs/components/Component_Renderable.hpp"

namespace engine::ecs::systems
{
	void CRenderSystem::Render(CEntityManager& _components, sf::RenderTarget& _target)
	{
		using namespace engine::ecs::components;

		_components.ForEach<SCoRenderable, SCoTransform>(
			[&_target](entity_id_t, SCoRenderable& _renderable, const SCoTransform& _transform)
			{
				if (const sf::Drawable* drawable = _renderable.GetDrawable())
				{
					sf::RenderStates states;
					states.transform.translate(_transform.Position);
					states.transform.rotate(_transform.Rotation);
					states.transform.scale(_transform.Scale);

					_target.draw(*drawable, states);
				}
			});
	}
}
