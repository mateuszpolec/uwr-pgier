#pragma once

#include <nlohmann/json.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Angle.hpp>
#include <SFML/Graphics/Color.hpp>

namespace sf
{
	inline void to_json(nlohmann::json& _j, const Vector2f& _v) { _j = nlohmann::json::array({ _v.x, _v.y }); }
	inline void from_json(const nlohmann::json& _j, Vector2f& _v) { _j.at(0).get_to(_v.x); _j.at(1).get_to(_v.y); }

	inline void to_json(nlohmann::json& _j, const Vector2i& _v) { _j = nlohmann::json::array({ _v.x, _v.y }); }
	inline void from_json(const nlohmann::json& _j, Vector2i& _v) { _j.at(0).get_to(_v.x); _j.at(1).get_to(_v.y); }

	inline void to_json(nlohmann::json& _j, const Vector2u& _v) { _j = nlohmann::json::array({ _v.x, _v.y }); }
	inline void from_json(const nlohmann::json& _j, Vector2u& _v) { _j.at(0).get_to(_v.x); _j.at(1).get_to(_v.y); }

	inline void to_json(nlohmann::json& _j, const Angle& _a) { _j = _a.asDegrees(); }
	inline void from_json(const nlohmann::json& _j, Angle& _a) { _a = degrees(_j.get<float>()); }

	inline void to_json(nlohmann::json& _j, const Color& _c) { _j = nlohmann::json::array({ _c.r, _c.g, _c.b, _c.a }); }
	inline void from_json(const nlohmann::json& _j, Color& _c)
	{
		_j.at(0).get_to(_c.r);
		_j.at(1).get_to(_c.g);
		_j.at(2).get_to(_c.b);
		_c.a = _j.size() > 3 ? _j.at(3).get<std::uint8_t>() : 255;
	}
}