#pragma once

#include <filesystem>
#include <memory>
#include <string>

#include <SFML/System/FileInputStream.hpp>

#include "engine/core/NonCopyable.hpp"
#include "AssetTraits.hpp"
#include "AssetTypes.hpp"
#include "engine/filesystem/Paths.hpp"

namespace engine::assets
{
	///////////////////////////////////////////////////////////////////////////////////////
	class IResource : public engine::core::NonCopyable
	{
		public:

			IResource(std::filesystem::path _absolute_path)
				: m_Filename(_absolute_path.filename().string())
				, m_RelativePath(std::filesystem::relative(_absolute_path, filesystem::Assets()).string())
				, m_AbsolutePath(_absolute_path.string())
			{
			}

			virtual ~IResource() = default;

			[[nodiscard]] virtual EAssetType GetType() const = 0;
			[[nodiscard]] virtual bool LoadFromStream(std::unique_ptr<sf::FileInputStream> _stream) = 0;
			[[nodiscard]] bool Load()
			{
				auto stream = std::make_unique<sf::FileInputStream>();
				if (!stream->open(m_AbsolutePath))
				{
					m_LoadState = EAssetLoadState::Failed;
					return false;
				}

				if (!LoadFromStream(std::move(stream)))
				{
					m_LoadState = EAssetLoadState::Failed;
					return false;
				}

				m_LoadState = EAssetLoadState::Loaded;
				return true;
			}

			[[nodiscard]] EAssetLoadState GetLoadState() const { return m_LoadState; }
			[[nodiscard]] const std::string& GetFilename() const { return m_Filename; }
			[[nodiscard]] const std::string& GetRelativePath() const { return m_RelativePath; }
			[[nodiscard]] const std::string& GetAbsolutePath() const { return m_AbsolutePath; }

		protected:

			EAssetLoadState m_LoadState = EAssetLoadState::Unloaded;

		private:

			std::string m_Filename;
			std::string m_RelativePath;
			std::string m_AbsolutePath;
	};

	///////////////////////////////////////////////////////////////////////////////////////
	template<typename T>
	class TResource : public IResource
	{
		public:

			TResource(std::filesystem::path _relative_path)
				: IResource(_relative_path)
			{
			}

			[[nodiscard]] EAssetType GetType() const override { return TAssetTraits<T>::TYPE; }

			[[nodiscard]] bool LoadFromStream(std::unique_ptr<sf::FileInputStream> _stream) override
			{
				const bool is_ok = TAssetTraits<T>::Load(m_Resource, *_stream);

				if (!is_ok)
				{
					m_LoadState = EAssetLoadState::Failed;
					return false;
				}

				if constexpr (TAssetTraits<T>::CACHE_STREAM)
				{
					m_Stream = std::move(_stream);
				}

				return true;
			}

			[[nodiscard]] T& Get() { return m_Resource; }
			[[nodiscard]] const T& Get() const { return m_Resource; }

		private:

			T m_Resource;
			std::unique_ptr<sf::FileInputStream> m_Stream;
	};
}