#include "Paths.hpp"

std::filesystem::path engine::filesystem::AppData()
{
	char* appdata_buffer = nullptr;
	size_t buffer_size = 0;

	const errno_t err = _dupenv_s(&appdata_buffer, &buffer_size, "APPDATA");

	if (err != 0 || appdata_buffer == nullptr)
	{
		if (appdata_buffer)
		{
			free(appdata_buffer);
		}

		throw std::runtime_error("Failed to retrieve APPDATA environment variable.");
	}

	std::filesystem::path appdata_path = std::filesystem::path(appdata_buffer);
	free(appdata_buffer);
	return appdata_path;
}
