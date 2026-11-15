#pragma once

#include <memory>
#include <variant>

#include <nlohmann/json.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Shape.hpp>
#include <SFML/Graphics/Texture.hpp>

#include "engine/assets/AssetHandle.hpp"

namespace engine::ecs::components
{
	struct SCoRenderable
	{
		static constexpr std::string_view JSON_NAME = "Renderable";
///////////////////////////////////////////////////////////////////////////////////////
		std::variant<std::unique_ptr<sf::Sprite>, std::unique_ptr<sf::Shape>> Renderable;

		engine::assets::TAssetHandle<sf::Texture> TextureHandle;

		std::string Layer;
		int LayerIndex = -1;

///////////////////////////////////////////////////////////////////////////////////////
		template<typename T, typename... Args>
		T* SetShape(Args&&... _args)
		{
			static_assert(std::is_base_of_v<sf::Shape, T>, "T must be derived from sf::Shape");

			Renderable = std::unique_ptr<sf::Shape>(std::make_unique<T>(std::forward<Args>(_args)...));
			return static_cast<T*>(GetAsShape());
		}

		sf::Sprite* SetSprite(engine::assets::TAssetHandle<sf::Texture> _texture_handle)
		{
			TextureHandle = std::move(_texture_handle);
			Renderable = std::make_unique<sf::Sprite>(TextureHandle.Get());
			return GetAsSprite();
		}

		[[nodiscard]] sf::Sprite* GetAsSprite()
		{
			auto* sprite = std::get_if<std::unique_ptr<sf::Sprite>>(&Renderable);
			return sprite ? sprite->get() : nullptr;
		}

		[[nodiscard]] sf::Shape* GetAsShape()
		{
			auto* shape = std::get_if<std::unique_ptr<sf::Shape>>(&Renderable);
			return shape ? shape->get() : nullptr;
		}

		[[nodiscard]] const sf::Drawable* GetDrawable() const
		{
			return std::visit([](const auto& _ptr) -> const sf::Drawable* { return _ptr.get(); }, Renderable);
		}

		[[nodiscard]] sf::FloatRect GetLocalBounds() const
		{
			return std::visit([](const auto& _ptr) -> sf::FloatRect
				{
					return _ptr ? _ptr->getLocalBounds() : sf::FloatRect{};
				}, Renderable);
		}

		void FindLayer();
	};

	void from_json(const nlohmann::json& _j, SCoRenderable& _component);
}