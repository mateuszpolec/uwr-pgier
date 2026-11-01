#pragma once

#include <memory>
#include <filesystem>

#include <spdlog/spdlog.h>

namespace engine::core
{
	class CLogger
	{
		public:

			static void Initialize();
			static void Shutdown();

			[[nodiscard]] static std::shared_ptr<spdlog::logger> GetLogger() { return s_Logger; }
			[[nodiscard]] static std::filesystem::path GetLogFilePath() { return s_LogFilePath; }
			[[nodiscard]] static std::filesystem::path GetLogDirectory() { return s_LogDirectory; }

		private:

			[[nodiscard]] static std::filesystem::path GetDefaultLogDirectory();
			[[nodiscard]] static std::filesystem::path GetDefaultLogFilePath();

		private:

			static std::shared_ptr<spdlog::logger> s_Logger;
			static std::filesystem::path s_LogFilePath;
			static std::filesystem::path s_LogDirectory;
	};
}

#define LOG_TRACE(...) ::engine::core::CLogger::GetLogger()->trace(__VA_ARGS__)
#define LOG_DEBUG(...) ::engine::core::CLogger::GetLogger()->debug(__VA_ARGS__)
#define LOG_INFO(...)  ::engine::core::CLogger::GetLogger()->info(__VA_ARGS__)
#define LOG_WARN(...)  ::engine::core::CLogger::GetLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...) ::engine::core::CLogger::GetLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) ::engine::core::CLogger::GetLogger()->critical(__VA_ARGS__)