#pragma once

#include <memory>

#include "Resource.hpp"
#include "AssetTypes.hpp"

namespace engine::assets
{
	template<typename T>
	class TAssetHandle
	{
		public:

			explicit TAssetHandle(std::shared_ptr<TResource<T>> resource)
				: m_Resource(resource)
			{
			}

			[[nodiscard]] bool IsValid() const { return m_Resource && m_Resource->GetLoadState() == EAssetLoadState::Loaded; }
			[[nodiscard]] std::shared_ptr<T> GetResource() const { return m_Resource; }

			[[nodiscard]] T& Get() const { return m_Resource->Get(); }
			[[nodiscard]] T* operator->() const { return m_Resource->Get(); }
			[[nodiscard]] T& operator*() const { return *m_Resource->Get(); }

		private:

			std::shared_ptr<TResource<T>> m_Resource;
	};

	using texture_handle_t = TAssetHandle<sf::Texture>;
	using font_handle_t = TAssetHandle<sf::Font>;
	using sound_handle_t = TAssetHandle<sf::SoundBuffer>;
}

