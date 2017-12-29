#include "AssetDB.h"

AssetDatabase * AssetDatabase::instance = nullptr;
Vulkan * AssetDatabase::vulkanApp = NULL;

AssetDatabase::AssetDatabase()
{
}

AssetDatabase *AssetDatabase::GetInstance()
{
	if (instance == nullptr)
		instance = new AssetDatabase();

	return instance;
}
