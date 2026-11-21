#include <string>

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>

#include "Component_Renderable.hpp"
#include "engine/filesystem/JsonSerializationSfml.hpp"
#include "engine/game/Game.hpp"
#include "engine/level/LevelManager.hpp"

#include "engine/core/Assert.hpp"

namespace engine::ecs::components
{
	void from_json(const nlohmann::json& _j, SCoRenderable& _component)
	{
		ASSERT(_j.contains("Layer"), "Renderable component must define a Layer!");
		_component.Layer = _j.value("Layer", _component.Layer);

		if (_j.contains("Texture"))
		{
			const std::string texture_path = _j["Texture"].get<std::string>();
			_component.SetSprite(engine::game::CGame::Get().GetAssetsManager().Get<sf::Texture>(texture_path));
			return;
		}

		const std::string shape = _j.value("Shape", "Rectangle");
		const sf::Color color = _j.value("Color", sf::Color::White);

		if (shape == "Rectangle")
		{
			const sf::Vector2f size = _j.value("Size", sf::Vector2f(10.f, 10.f));
			auto* rectangle_shape = _component.SetShape<sf::RectangleShape>(size);
			rectangle_shape->setFillColor(color);
		}
		else if (shape == "Circle")
		{
			const float radius = _j.value("Radius", 50.f);
			auto* circle_shape = _component.SetShape<sf::CircleShape>(radius);
			circle_shape->setFillColor(color);
		}
		else
		{
			ASSERT(false, "Unknown shape type: " + shape);
		}
	}

	void SCoRenderable::FindLayer()
	{
		if (Layer.empty())
		{
			ASSERT(false, "Renderable component must define a Layer!");
			return;
		}

		if (LayerIndex != -1)
		{
			return;
		}

		const engine::level::CLevelManager& level_manager = engine::game::CGame::Get().GetLevelManager();
		if (!level_manager.HasLevel())
		{
			return;
		}

		LayerIndex = level_manager.GetInfo().FindLayerIndex(Layer);
		if (LayerIndex == -1)
		{
			ASSERT(false, "Renderable component has an invalid Layer: " + Layer);
			LayerIndex = 0;
		}
	}
}
