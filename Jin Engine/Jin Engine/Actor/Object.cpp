#include "Object.h"

#include "../Asset/AssetDB.h"

void Object::initialize(Vulkan *pvulkanApp, std::string actorNameParam, std::string pathParam, bool needUflipCorrection)
{
	vulkanApp = pvulkanApp;
	actorName = actorNameParam;

	UflipCorrection = needUflipCorrection;

	LoadFromFilename(pathParam);

	createObjectBuffer();
}

bool Object::LoadFromFilename(std::string path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | aiProcess_GenSmoothNormals | aiProcess_ValidateDataStructure);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
		return false;
	}

	int geomID = 0;

	processNode(scene->mRootNode, scene, geomID);

	numMaterials = scene->mNumMaterials;

	materialGroup.resize(numMaterials);

	for (size_t i = 0; i < geoms.size(); i++)
	{
		(materialGroup[geoms[i]->getMaterialID()]).push_back(static_cast<uint32_t>(i));
	}

	/*
	Culling.resize(geoms.size());

	for (size_t i = 0; i < geoms.size(); i++)
	{
		Culling[i] = false;
	}
	*/

	setAABB();

	return true;
}

void Object::createObjectBuffer()
{
	vulkanApp->createBuffer(sizeof(objectBuffer), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformObjectBuffer, uniformObjectBufferMemory);
	updateObjectBuffer();
}

void Object::updateObjectBuffer()
{
	Actor::update();
	objectBufferInfo.modelMat = modelMat;
	objectBufferInfo.InvTransposeMat = InvTransposeMat;

	/*
	glm::mat4 A = objectBufferInfo.modelMat;
	A[3] = glm::vec4(0, 0, 0, 1);
	objectBufferInfo.InvTransposeMat = glm::transpose(glm::inverse(A));
	*/

	vulkanApp->updateBuffer(&objectBufferInfo, uniformObjectBufferMemory, sizeof(objectBuffer));
}

void Object::processNode(aiNode* node, const aiScene const* scene, int &geomID)
{
	//process each mesh located at current node
	for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
		//the node object only contains indices to retrieve te mesh out of the main mMeshes array in scene
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

		std::string geomName = mesh->mName.C_Str();
		geomName += "_" + actorName + "_" + std::to_string(geomID);

		geomID++;

		Geometry* tempGeom = AssetDatabase::GetInstance()->SaveGeomtery(geomName);

		// = new Geometry;
		tempGeom->initialize(vulkanApp, geomName, UflipCorrection, mesh);
		geoms.push_back(tempGeom);

		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	}

	//process the children of this node
	for (unsigned int i = 0; i < node->mNumChildren; ++i) {
		processNode(node->mChildren[i], scene, geomID);
	}
}

void Object::connectMaterial(uint32_t matIndex)
{

}

void Object::setAABB()
{
	AABB = geoms[0]->AABB;

	geomAABB.push_back(AABB);

	for (size_t k = 1; k < geoms.size(); k++)
	{
		AABB.maxPt = glm::max(AABB.maxPt, geoms[k]->AABB.maxPt);
		AABB.minPt = glm::min(AABB.minPt, geoms[k]->AABB.minPt);

		geomAABB.push_back(geoms[k]->AABB);
	}

	AABB.Center = (AABB.maxPt + AABB.minPt) * 0.5f;
	AABB.Extents = glm::abs(AABB.maxPt - AABB.minPt) * 0.5f;

	//AABB.radius = glm::length(glm::vec3(AABB.Extents));

	AABB.corners[0] = AABB.Center + glm::vec4(-AABB.Extents);
	AABB.corners[1] = AABB.Center + glm::vec4(AABB.Extents.x, -AABB.Extents.y, -AABB.Extents.z, 0.0);
	AABB.corners[2] = AABB.Center + glm::vec4(-AABB.Extents.x, AABB.Extents.y, -AABB.Extents.z, 0.0);
	AABB.corners[3] = AABB.Center + glm::vec4(-AABB.Extents.x, -AABB.Extents.y, AABB.Extents.z, 0.0);

	AABB.corners[4] = AABB.Center + glm::vec4(-AABB.Extents.x, AABB.Extents.y, AABB.Extents.z, 0.0);
	AABB.corners[5] = AABB.Center + glm::vec4(AABB.Extents.x, -AABB.Extents.y, AABB.Extents.z, 0.0);
	AABB.corners[6] = AABB.Center + glm::vec4(AABB.Extents.x, AABB.Extents.y, -AABB.Extents.z, 0.0);
	AABB.corners[7] = AABB.Center + glm::vec4(AABB.Extents);
}