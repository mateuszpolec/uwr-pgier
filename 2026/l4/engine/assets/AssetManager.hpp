#pragma once

#include <filesystem>
#include <memory>
#include <unordered_map>
#include <string>

#include "engine/core/NonCopyable.hpp"
#include "AssetTypes.hpp"
#include "AssetHandle.hpp"
#include "Resource.hpp"

namespace engine::assets
{
	class CAssetsManager : public engine::core::NonCopyable
	{
		public:

			void Load(const std::filesystem::path& _path, bool _recursivly, bool _skip_already_loaded);
			size_t Size() const { return m_Resources.size(); }

			template<typename T>
			TAssetHandle<T> Get(const std::string& _path) const
			{
				if (const auto it = m_Resources.find(_path); it != m_Resources.end())
				{
					if (const auto resource = std::dynamic_pointer_cast<TResource<T>>(it->second))
					{
						return TAssetHandle<T>(resource);
					}
				}

				return TAssetHandle<T>(nullptr);
			}

			[[nodiscard]] bool Contains(const std::string& _path) const;

		private:

			template<typename T>
			[[nodiscard]] bool Load(const std::filesystem::path& _path)
			{
				auto resource = std::make_shared<TResource<T>>(_path);

				if (!resource->Load())
				{
					return false;
				}

				m_Resources[resource->GetRelativePath()] = resource;
				return true;
			}

			[[nodiscard]] static EAssetType DetectAssetType(const std::filesystem::path& _path);

		private:

			std::unordered_map<std::string, std::shared_ptr<IResource>> m_Resources;
	};
}