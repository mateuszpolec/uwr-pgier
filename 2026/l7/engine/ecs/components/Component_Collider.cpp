#include "Component_Collider.hpp"

#include "engine/core/Logger.hpp"

namespace engine::ecs::components
{
	void from_json(const nlohmann::json& _j, SCoCollider& _component)
	{
		using namespace engine::physics;

		_component.Offset = _j.value("Offset", _component.Offset);
		_component.Layer = _j.value("Layer", _component.Layer);

		const auto shape = _j.find("Shape");
		if (shape == _j.end())
		{
			return;
		}

		const std::string type = shape->value("Type", std::string(SAabbShape::JSON_NAME));
		if (type == SAabbShape::JSON_NAME)
		{
			_component.Shape = shape->get<SAabbShape>();
		}
		else
		{
			LOG_WARN("Unknown collider shape type '{}'", type);
		}
	}
}