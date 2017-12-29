#pragma once

#include "../Core/Vulkan.h"

static std::vector<char> readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();
	return buffer;
}

class Asset
{
public:
	Asset() {};

	// Not all assets must implement this, but if you want it to be cached you must
	virtual void LoadFromFilename(Vulkan *vulkanAppParam, std::string filename) = 0;
	virtual ~Asset() {};
	virtual void shutDown() = 0;

	Vulkan *vulkanApp;
	std::string path;
};