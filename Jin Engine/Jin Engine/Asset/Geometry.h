#pragma once

#include "Asset.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct SimpleVertex
{
	glm::vec4 positions;

	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription = {};

		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(SimpleVertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 1> getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 1> attributeDescriptions = {};
		
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(SimpleVertex, positions);

		return attributeDescriptions;
	}
};

struct Vertex
{
	glm::vec4 positions;
	glm::vec4 colors;

	glm::vec4 tangents;
	glm::vec4 bitangents;
	glm::vec4 normals;

	glm::vec2 texcoords;

	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 6> getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 6> attributeDescriptions = {};
		//pos
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, positions);
		//col
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, colors);
		//tan
		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, tangents);
		//bitan
		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(Vertex, bitangents);
		//nor
		attributeDescriptions[4].binding = 0;
		attributeDescriptions[4].location = 4;
		attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[4].offset = offsetof(Vertex, normals);
		//uv
		attributeDescriptions[5].binding = 0;
		attributeDescriptions[5].location = 5;
		attributeDescriptions[5].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[5].offset = offsetof(Vertex, texcoords);

		return attributeDescriptions;
	}
};

class Geometry : public Asset
{
public:
	Geometry():UflipCorrection(false)
	{

	}
	~Geometry()
	{
		shutDown();
	}

	void shutDown();

	virtual void LoadFromFilename(Vulkan *vulkanAppParam, std::string filename) {};

	void initialize(Vulkan *pvulkanApp, std::string pathParam, bool needUflipCorrection, const aiMesh* mesh);
	void setGeometry(const aiMesh* mesh);

	void createVertexBuffer();

	void createIndexBuffer();

	void createTBN();

	void fillTBN();

	VkBuffer getVertexBuffer();

	VkBuffer getIndexBuffer();
	uint32_t getIndexCount()
	{
		return static_cast<uint32_t>(indices.size());
	}
	/*
	BoundingBox getAABB()
	{
		return AABB;
	}
	*/
	uint32_t getMaterialID()
	{
		return materialID;
	}

	BoundingBox AABB;

private:

	std::vector<glm::vec3> Vpositions;

	std::vector<glm::vec3> Vtangent;
	std::vector<glm::vec3> Vbinormal;

	std::vector<glm::vec3> Vnormals;
	std::vector<glm::vec2> Vuvs;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<std::pair<int, int>> handness;

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;

	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	

	uint32_t numVetices;
	uint32_t numTriangles;

	uint32_t materialID;

	bool UflipCorrection;
};
