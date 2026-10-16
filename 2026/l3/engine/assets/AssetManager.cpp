#include <algorithm>

#include "AssetManager.hpp"
#include "engine/core/Logger.hpp"

namespace engine::assets
{
	void CAssetsManager::Load(const std::filesystem::path& _path, bool _recursivly, bool _skip_already_loaded)
	{
		if (!std::filesystem::exists(_path))
		{
			LOG_WARN("AssetsManager: Path does not exist: {}", _path.string());
			return;
		}

		if (std::filesystem::is_directory(_path))
		{
			for (const auto& entry : std::filesystem::directory_iterator(_path))
			{
				if (entry.is_directory() && _recursivly)
				{
					Load(entry.path(), _recursivly, _skip_already_loaded);
				}
				else if (entry.is_regular_file())
				{
					if (_skip_already_loaded)
					{
						const std::string relative_path = std::filesystem::relative(entry.path(), filesystem::Assets()).string();
						if (Contains(relative_path))
						{
							continue;
						}
					}

					const EAssetType asset_type = DetectAssetType(entry.path());
					switch (asset_type)
					{
						case EAssetType::Texture:
						{
							const bool ok = Load<sf::Texture>(entry.path());
							if (!ok)
							{
								LOG_WARN("AssetsManager: Failed to load texture: {}", entry.path().string());
							}

							break;
						}
						case EAssetType::Font:
						{
							const bool ok = Load<sf::Font>(entry.path());
							if (!ok)
							{
								LOG_WARN("AssetsManager: Failed to load font: {}", entry.path().string());
							}
							break;
						}
						case EAssetType::Sound:
						{
							const bool ok = Load<sf::SoundBuffer>(entry.path());
							if (!ok)
							{
								LOG_WARN("AssetsManager: Failed to load sound: {}", entry.path().string());
							}

							break;
						}
						default:
						{
							LOG_WARN("AssetsManager: Unsupported asset type for file: {}", entry.path().string());
							break;
						}
					}
				}
			}
		}
		else if (std::filesystem::is_regular_file(_path))
		{
			if (_skip_already_loaded)
			{
				const std::string relative_path = std::filesystem::relative(_path, filesystem::Assets()).string();
				if (Contains(relative_path))
				{
					return;
				}
			}

			const EAssetType asset_type = DetectAssetType(_path);
			switch (asset_type)
			{
				case EAssetType::Texture:
				{
					const bool ok = Load<sf::Texture>(_path);
					if (!ok)
					{
						LOG_WARN("AssetsManager: Failed to load texture: {}", _path.string());
					}

					break;
				}
				case EAssetType::Font:
				{
					const bool ok = Load<sf::Font>(_path);
					if (!ok)
					{
						LOG_WARN("AssetsManager: Failed to load font: {}", _path.string());
					}

					break;
				}
				case EAssetType::Sound:
				{
					const bool ok = Load<sf::SoundBuffer>(_path);
					if (!ok)
					{
						LOG_WARN("AssetsManager: Failed to load sound: {}", _path.string());
					}

					break;
				}
				default:
				{
					LOG_WARN("AssetsManager: Unsupported asset type for file: {}", _path.string());
					break;
				}
			}
		}
		else
		{
			LOG_WARN("AssetsManager: Path is neither a directory nor a regular file: {}", _path.string());
		}
	}

	bool CAssetsManager::Contains(const std::string& _path) const
	{
		return m_Resources.find(_path) != m_Resources.end();
	}

	EAssetType CAssetsManager::DetectAssetType(const std::filesystem::path& _path)
	{
		std::string extension = _path.extension().string();
		std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

		const auto fn_extension_match = [&extension](const auto& _extensions) { return std::ranges::find(_extensions, extension) != _extensions.end(); };

		if (fn_extension_match(TAssetTraits<sf::Texture>::EXTENSIONS))
		{
			return EAssetType::Texture;
		}
		else if (fn_extension_match(TAssetTraits<sf::Font>::EXTENSIONS))
		{
			return EAssetType::Font;
		}
		else if (fn_extension_match(TAssetTraits<sf::SoundBuffer>::EXTENSIONS))
		{
			return EAssetType::Sound;
		}

		return EAssetType::Unknown;
	}
}