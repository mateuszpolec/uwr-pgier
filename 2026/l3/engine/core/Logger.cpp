#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <vector>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#if defined(WINDOWS)
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <Windows.h>
#include <Shlobj.h>
#endif

#include "Logger.hpp"

#include "engine/filesystem/Paths.hpp"

namespace engine::core
{
	std::shared_ptr<spdlog::logger> CLogger::s_Logger;
	std::filesystem::path CLogger::s_LogFilePath;
	std::filesystem::path CLogger::s_LogDirectory;

	void CLogger::Initialize()
	{
		if (s_Logger)
		{
			return;
		}

		s_LogDirectory = GetDefaultLogDirectory();

		if (std::filesystem::exists(s_LogDirectory))
		{
			std::filesystem::create_directories(s_LogDirectory);
		}

		s_LogFilePath = GetDefaultLogFilePath();

		std::vector<spdlog::sink_ptr> sinks;

		auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
		sinks.push_back(console_sink);

		auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(s_LogFilePath.string(), true);
		file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
		sinks.push_back(file_sink);

		s_Logger = std::make_shared<spdlog::logger>("uwr_logger", sinks.begin(), sinks.end());

#if defined(DEBUG)
		s_Logger->set_level(spdlog::level::trace);
#else
		s_Logger->set_level(spdlog::level::info);
#endif

		s_Logger->flush_on(spdlog::level::warn);

		spdlog::register_logger(s_Logger);
		spdlog::set_default_logger(s_Logger);

		LOG_INFO("Logger initialized. Log file: {}", s_LogFilePath.string());
	}

	void CLogger::Shutdown()
	{
		if (s_Logger)
		{
			LOG_INFO("Shutting down logger.");
			s_Logger->flush();
			spdlog::shutdown();
			s_Logger.reset();
		}
	}

	std::filesystem::path CLogger::GetDefaultLogDirectory()
	{
		std::filesystem::path log_directory = engine::filesystem::AppData() / "uwr" / "logs";
		return log_directory;
	}

	std::filesystem::path CLogger::GetDefaultLogFilePath()
	{
		const auto now = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
		return s_LogDirectory / std::format("log_{:%Y-%m-%d_%H-%M-%S}.txt", std::chrono::floor<std::chrono::seconds>(now));
	}
}