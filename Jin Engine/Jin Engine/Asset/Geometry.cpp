#include "Geometry.h"

void Geometry::shutDown()
{
	vkDestroyBuffer(vulkanApp->getDevice(), vertexBuffer, nullptr);
	vkFreeMemory(vulkanApp->getDevice(), vertexBufferMemory, nullptr);

	vkDestroyBuffer(vulkanApp->getDevice(), indexBuffer, nullptr);
	vkFreeMemory(vulkanApp->getDevice(), indexBufferMemory, nullptr);
}

void Geometry::initialize(Vulkan *pvulkanApp, std::string pathParam, bool needUflipCorrection, const aiMesh* mesh)
{
	vulkanApp = pvulkanApp;
	path = pathParam;
	UflipCorrection = needUflipCorrection;

	setGeometry(mesh);
	//createTBN();
	fillTBN();

	createVertexBuffer();
	createIndexBuffer();
}

void Geometry::fillTBN()
{
	for (size_t i = 0; i < numTriangles; i++)
	{
		glm::vec3 localPos[3];
		glm::vec2 uv[3];

		unsigned int index01 = indices[3 * i];
		unsigned int index02 = indices[3 * i + 1];
		unsigned int index03 = indices[3 * i + 2];

		{
			localPos[0] = vertices[index01].positions;
			localPos[1] = vertices[index02].positions;
			localPos[2] = vertices[index03].positions;

			uv[0] = vertices[index01].texcoords;
			uv[1] = vertices[index02].texcoords;
			uv[2] = vertices[index03].texcoords;

			float u0 = uv[1].x - uv[0].x;
			float u1 = uv[2].x - uv[0].x;

			float v0 = uv[1].y - uv[0].y;
			float v1 = uv[2].y - uv[0].y;

			float dino = u0 * v1 - v0 * u1;

			glm::vec3 Pos1 = localPos[1] - localPos[0];
			glm::vec3 Pos2 = localPos[2] - localPos[0];

			glm::vec2 UV1 = uv[1] - uv[0];
			glm::vec2 UV2 = uv[2] - uv[0];

			glm::vec3 tan;
			glm::vec3 bit;
			glm::vec3 nor;


			if (dino != 0.0f)
			{
				tan = glm::normalize((UV2.y * Pos1 - UV1.y * Pos2) / dino);
				bit = glm::normalize((Pos2 - UV2.x * tan) / UV2.y);
				nor = glm::normalize(glm::cross(tan, bit));

				// Calculate handedness
				
				glm::vec3 fFaceNormal = glm::normalize(glm::cross(Pos2, Pos1));

				if (glm::isnan(vertices[index01].tangents.x) || glm::isinf(vertices[index01].tangents.x) ||
					glm::isnan(vertices[index02].tangents.x) || glm::isinf(vertices[index02].tangents.x) ||
					glm::isnan(vertices[index03].tangents.x) || glm::isinf(vertices[index03].tangents.x))
				{
					//U flip
					if (glm::dot(nor, fFaceNormal) < 0.0f)
					{
						tan = -(tan);
					}


					if (glm::isnan(vertices[index01].tangents.x) || glm::isinf(vertices[index01].tangents.x))
					{
						glm::vec3 localtan = tan;
						glm::vec3 localbit = bit;

						if (!(glm::isnan(vertices[index02].tangents.x) || glm::isinf(vertices[index02].tangents.x)))
						{
							localtan = vertices[index02].tangents;
						}
						else if (!(glm::isnan(vertices[index03].tangents.x) || glm::isinf(vertices[index03].tangents.x)))
						{
							localtan = vertices[index03].tangents;
						}

						nor = glm::vec3(vertices[index01].normals);
						localbit = glm::cross(nor, localtan);
						localtan = glm::cross(localbit, nor);

						vertices[index01].bitangents = glm::vec4(localbit, 0.0);
						vertices[index01].tangents = glm::vec4(localtan, 0.0);
					}

					if (glm::isnan(vertices[index02].tangents.x) || glm::isinf(vertices[index02].tangents.x))
					{
						glm::vec3 localtan = tan;
						glm::vec3 localbit = bit;

						if (!(glm::isnan(vertices[index01].tangents.x) || glm::isinf(vertices[index01].tangents.x)))
						{
							localtan = vertices[index01].tangents;
						}
						else if (!(glm::isnan(vertices[index03].tangents.x) || glm::isinf(vertices[index03].tangents.x)))
						{
							localtan = vertices[index03].tangents;
						}

						nor = glm::vec3(vertices[index02].normals);
						localbit = glm::cross(nor, localtan);
						localtan = glm::cross(localbit, nor);

						vertices[index02].bitangents = glm::vec4(localbit, 0.0);
						vertices[index02].tangents = glm::vec4(localtan, 0.0);
					}

					if (glm::isnan(vertices[index03].tangents.x) || glm::isinf(vertices[index03].tangents.x))
					{
						glm::vec3 localtan = tan;
						glm::vec3 localbit = bit;

						if (!(glm::isnan(vertices[index02].tangents.x) || glm::isinf(vertices[index02].tangents.x)))
						{
							localtan = vertices[index02].tangents;
						}
						else if (!(glm::isnan(vertices[index01].tangents.x) || glm::isinf(vertices[index01].tangents.x)))
						{
							localtan = vertices[index01].tangents;
						}

						nor = glm::vec3(vertices[index03].normals);
						localbit = glm::cross(nor, localtan);
						localtan = glm::cross(localbit, nor);

						vertices[index03].bitangents = glm::vec4(localbit, 0.0);
						vertices[index03].tangents = glm::vec4(localtan, 0.0);
					}
				}
				else
				{
					//U flip
					if (glm::dot(nor, fFaceNormal) < 0.0f)
					{
						vertices[index01].tangents = -vertices[index01].tangents;
						vertices[index02].tangents = -vertices[index02].tangents;
						vertices[index03].tangents = -vertices[index03].tangents;

						vertices[index01].bitangents = glm::vec4(glm::cross(glm::vec3(vertices[index01].normals), glm::vec3(vertices[index01].tangents)), 0.0);
						vertices[index02].bitangents = glm::vec4(glm::cross(glm::vec3(vertices[index02].normals), glm::vec3(vertices[index02].tangents)), 0.0);
						vertices[index03].bitangents = glm::vec4(glm::cross(glm::vec3(vertices[index03].normals), glm::vec3(vertices[index03].tangents)), 0.0);

						/*
						vertices[index01].tangents = glm::vec4(0.0);
						vertices[index02].tangents = glm::vec4(0.0);
						vertices[index03].tangents = glm::vec4(0.0);
						*/
					}

				}
				
						
			}
			else
			{
				
				glm::vec3 up = glm::normalize(glm::vec3(0.001, 1, 0.001));
				glm::vec3 surftan;
				glm::vec3 surfbinor;


				if (glm::isnan(vertices[index01].tangents.x) || glm::isinf(vertices[index01].tangents.x))
				{
					surftan = glm::normalize(glm::cross(up, glm::vec3(vertices[index01].normals)));
					surfbinor = glm::cross(glm::vec3(vertices[index01].normals), surftan);

					vertices[index01].tangents = glm::vec4(surftan, 0.0f);
					vertices[index01].bitangents = glm::vec4(surfbinor, 0.0f);
				}

				if (glm::isnan(vertices[index02].tangents.x) || glm::isinf(vertices[index02].tangents.x))
				{
					surftan = glm::normalize(glm::cross(up, glm::vec3(vertices[index02].normals)));
					surfbinor = glm::cross(glm::vec3(vertices[index02].normals), surftan);

					vertices[index02].tangents = glm::vec4(surftan, 0.0f);
					vertices[index02].bitangents = glm::vec4(surfbinor, 0.0f);
				}

				if (glm::isnan(vertices[index03].tangents.x) || glm::isinf(vertices[index03].tangents.x))
				{
					surftan = glm::normalize(glm::cross(up, glm::vec3(vertices[index03].normals)));
					surfbinor = glm::cross(glm::vec3(vertices[index03].normals), surftan);

					vertices[index03].tangents = glm::vec4(surftan, 0.0f);
					vertices[index03].bitangents = glm::vec4(surfbinor, 0.0f);
				}
			}
		}
	}
}

void Geometry::setGeometry(const aiMesh* mesh)
{
	this->numVetices = mesh->mNumVertices;

	for (unsigned int j = 0; j < this->numVetices; j++)
	{
		this->Vpositions.push_back(glm::vec3(mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z));

		this->Vtangent.push_back(glm::vec3(mesh->mTangents[j].x, mesh->mTangents[j].y, mesh->mTangents[j].z));
		this->Vbinormal.push_back(glm::vec3(mesh->mBitangents[j].x, mesh->mBitangents[j].y, mesh->mBitangents[j].z));

		this->Vnormals.push_back(glm::vec3(mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z));
		this->Vuvs.push_back(glm::vec3(mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y, mesh->mTextureCoords[0][j].z));
	}

	glm::vec4 maxCorner = glm::vec4(-FLT_MAX);
	glm::vec4 minCorner = glm::vec4(FLT_MAX);;

	for (unsigned int j = 0; j < this->numVetices; j++)
	{
		Vertex tempVertexInfo;
		tempVertexInfo.positions = glm::vec4(this->Vpositions[j], 0.0f);
		tempVertexInfo.colors = glm::vec4(0.0f);
		
		tempVertexInfo.normals = glm::vec4(this->Vnormals[j], 0.0f);
		tempVertexInfo.tangents = glm::vec4(this->Vtangent[j], 0.0f);
		tempVertexInfo.bitangents = glm::vec4(this->Vbinormal[j], 0.0f);
		
		tempVertexInfo.texcoords = this->Vuvs[j];
		tempVertexInfo.tangents = glm::vec4(this->Vtangent[j], 0.0f);
		tempVertexInfo.bitangents = glm::vec4(this->Vbinormal[j], 0.0f);

		/*
		if (glm::isnan(tempVertexInfo.tangents.x) || glm::isinf(tempVertexInfo.tangents.x))
		{
			glm::vec3 up;

			glm::vec3 surftan;
			glm::vec3 surfbinor;		

			surftan = glm::normalize(glm::cross(up, glm::vec3(tempVertexInfo.normals)));
			surfbinor = glm::cross(glm::vec3(tempVertexInfo.normals), surftan);
			
			//glm::normalize(surftan * normap.x + surfbinor * normap.y + geomnor * normap.z);

			tempVertexInfo.tangents = glm::vec4(surftan, 0.0f);
			tempVertexInfo.bitangents = glm::vec4(surfbinor, 0.0f);
			
		}
		else
		{
			tempVertexInfo.tangents = glm::vec4(this->Vtangent[j], 0.0f);
			tempVertexInfo.bitangents = glm::vec4(this->Vbinormal[j], 0.0f);
		}
		*/

		//tempVertexInfo.tangents = glm::vec4(0.0f);
		//tempVertexInfo.bitangents = glm::vec4(0.0f);

		this->vertices.push_back(tempVertexInfo);

		maxCorner = glm::max(maxCorner, tempVertexInfo.positions);
		minCorner = glm::min(minCorner, tempVertexInfo.positions);
	}
	
	AABB.maxPt = maxCorner;
	AABB.minPt = minCorner;

	AABB.Center = (maxCorner + minCorner) * 0.5f;
	AABB.Extents = glm::abs(maxCorner - minCorner) * 0.5f;
	
	AABB.corners[0] = AABB.Center + glm::vec4(-AABB.Extents);
	AABB.corners[1] = AABB.Center + glm::vec4(AABB.Extents.x, -AABB.Extents.y, -AABB.Extents.z, 0.0);
	AABB.corners[2] = AABB.Center + glm::vec4(-AABB.Extents.x, AABB.Extents.y, -AABB.Extents.z, 0.0);
	AABB.corners[3] = AABB.Center + glm::vec4(-AABB.Extents.x, -AABB.Extents.y, AABB.Extents.z, 0.0);

	AABB.corners[4] = AABB.Center + glm::vec4(-AABB.Extents.x, AABB.Extents.y, AABB.Extents.z, 0.0);
	AABB.corners[5] = AABB.Center + glm::vec4(AABB.Extents.x, -AABB.Extents.y, AABB.Extents.z, 0.0);
	AABB.corners[6] = AABB.Center + glm::vec4(AABB.Extents.x, AABB.Extents.y, -AABB.Extents.z, 0.0);
	AABB.corners[7] = AABB.Center + glm::vec4(AABB.Extents);


	//AABB.radius = glm::length(glm::vec3(AABB.Extents));

	//indices
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; ++j)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	this->numTriangles = (unsigned int)this->indices.size() / 3;

	materialID = mesh->mMaterialIndex;
}

void Geometry::createVertexBuffer()
{
	VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();

	vulkanApp->createBuffer(sizeof(Vertex) * vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBuffer, vertexBufferMemory);

	void* data;
	vkMapMemory(vulkanApp->getDevice(), vertexBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(vulkanApp->getDevice(), vertexBufferMemory);
}

void Geometry::createIndexBuffer()
{
	VkDeviceSize bufferSize = sizeof(uint32_t) * indices.size();

	vulkanApp->createBuffer(sizeof(uint32_t) * indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, indexBuffer, indexBufferMemory);

	void* data;
	vkMapMemory(vulkanApp->getDevice(), indexBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t)bufferSize);
	vkUnmapMemory(vulkanApp->getDevice(), indexBufferMemory);
}

VkBuffer Geometry::getVertexBuffer()
{
	return vertexBuffer;
}

VkBuffer Geometry::getIndexBuffer()
{
	return indexBuffer;
}

void Geometry::createTBN()
{
	std::pair<int, int> tempPair;
	tempPair.first = -1;
	tempPair.second = -1;
	handness.resize(vertices.size(), tempPair);

	for (size_t i = 0; i < numTriangles; i++)
	{
		glm::vec3 localPos[3];
		glm::vec2 uv[3];

		unsigned int index01 = indices[3 * i];
		unsigned int index02 = indices[3 * i + 1];
		unsigned int index03 = indices[3 * i + 2];

		localPos[0] = vertices[index01].positions;
		localPos[1] = vertices[index02].positions;
		localPos[2] = vertices[index03].positions;

		uv[0] = vertices[index01].texcoords;
		uv[1] = vertices[index02].texcoords;
		uv[2] = vertices[index03].texcoords;



		float u0 = uv[1].x - uv[0].x;
		float u1 = uv[2].x - uv[0].x;

		float v0 = uv[1].y - uv[0].y;
		float v1 = uv[2].y - uv[0].y;

		float dino = u0 * v1 - v0 * u1;

		glm::vec3 Pos1 = localPos[1] - localPos[0];
		glm::vec3 Pos2 = localPos[2] - localPos[0];

		glm::vec2 UV1 = uv[1] - uv[0];
		glm::vec2 UV2 = uv[2] - uv[0];

		glm::vec3 tan;
		glm::vec3 bit;
		glm::vec3 nor;


		if (dino != 0.0f)
		{
			tan = glm::normalize((UV2.y * Pos1 - UV1.y * Pos2) / dino);
			bit = glm::normalize((Pos2 - UV2.x * tan) / UV2.y);
			nor = glm::normalize(glm::cross(tan, bit));

			// Calculate handedness
			glm::vec3 fFaceNormal = glm::normalize(glm::cross(Pos2, Pos1));

			//U flip
			if (glm::dot(nor, fFaceNormal) < 0.0f)
			{
				tan = -(tan);

				if (handness[index01].first == -1)
				{
					handness[index01].first = 0;
				}

				if (handness[index02].first == -1)
				{
					handness[index02].first = 0;
				}

				if (handness[index03].first == -1)
				{
					handness[index03].first = 0;
				}

				if (handness[index01].first == 1)
				{
					//없으면 생성
					if (handness[index01].second == -1)
					{
						vertices.push_back(vertices[index01]);

						vertices[vertices.size() - 1].tangents = glm::vec4(0.0f);
						vertices[vertices.size() - 1].bitangents = glm::vec4(0.0f);

						std::pair<int, int> newHandness;
						newHandness.first = 0;
						newHandness.second = -1;
						handness.push_back(newHandness);

						handness[index01].second = (int)vertices.size() - 1;
					}

					//connect new indice
					indices[3 * i] = handness[index01].second;
					index01 = indices[3 * i];

					vertices[index01].colors.y = 1.0f;
				}

				if (handness[index02].first == 1)
				{
					//없으면 생성
					if (handness[index02].second == -1)
					{
						vertices.push_back(vertices[index02]);

						vertices[vertices.size() - 1].tangents = glm::vec4(0.0f);
						vertices[vertices.size() - 1].bitangents = glm::vec4(0.0f);

						std::pair<int, int> newHandness;
						newHandness.first = 0;
						newHandness.second = -1;
						handness.push_back(newHandness);

						handness[index02].second = (int)vertices.size() - 1;
					}

					//connect new indice
					indices[3 * i + 1] = handness[index02].second;
					index02 = indices[3 * i + 1];

					vertices[index02].colors.y = 1.0f;
				}

				if (handness[index03].first == 1)
				{
					//없으면 생성
					if (handness[index03].second == -1)
					{
						vertices.push_back(vertices[index03]);

						vertices[vertices.size() - 1].tangents = glm::vec4(0.0f);
						vertices[vertices.size() - 1].bitangents = glm::vec4(0.0f);

						std::pair<int, int> newHandness;
						newHandness.first = 0;
						newHandness.second = -1;
						handness.push_back(newHandness);

						handness[index03].second = (int)vertices.size() - 1;
					}

					//connect new indice
					indices[3 * i + 2] = handness[index03].second;
					index03 = indices[3 * i + 2];

					vertices[index03].colors.y = 1.0f;
				}

				if (vertices[index01].colors.y == 1.0f || vertices[index02].colors.y == 1.0f || vertices[index03].colors.y == 1.0f)
				{
					vertices[index01].colors.y = 1.0f;
					vertices[index02].colors.y = 1.0f;
					vertices[index03].colors.y = 1.0f;
				}

				vertices[index01].colors.x = 1.0f;
				vertices[index02].colors.x = 1.0f;
				vertices[index03].colors.x = 1.0f;
			}
			else
			{
				if (handness[index01].first == -1)
				{
					handness[index01].first = 1;
				}

				if (handness[index02].first == -1)
				{
					handness[index02].first = 1;
				}

				if (handness[index03].first == -1)
				{
					handness[index03].first = 1;
				}

				if (handness[index01].first == 0)
				{
					//없으면 생성
					if (handness[index01].second == -1)
					{
						vertices.push_back(vertices[index01]);

						vertices[vertices.size() - 1].tangents = glm::vec4(0.0f);
						vertices[vertices.size() - 1].bitangents = glm::vec4(0.0f);

						std::pair<int, int> newHandness;
						newHandness.first = 1;
						newHandness.second = -1;
						handness.push_back(newHandness);

						handness[index01].second = (int)vertices.size() - 1;
					}

					//connect new indice
					indices[3 * i] = handness[index01].second;
					index01 = indices[3 * i];
					vertices[index01].colors.z = 1.0f;
				}

				if (handness[index02].first == 0)
				{
					//없으면 생성
					if (handness[index02].second == -1)
					{
						vertices.push_back(vertices[index02]);

						vertices[vertices.size() - 1].tangents = glm::vec4(0.0f);
						vertices[vertices.size() - 1].bitangents = glm::vec4(0.0f);

						std::pair<int, int> newHandness;
						newHandness.first = 1;
						newHandness.second = -1;
						handness.push_back(newHandness);

						handness[index02].second = (int)vertices.size() - 1;
					}

					//connect new indice
					indices[3 * i + 1] = handness[index02].second;
					index02 = indices[3 * i + 1];
					vertices[index02].colors.z = 1.0f;
				}

				if (handness[index03].first == 0)
				{
					//없으면 생성
					if (handness[index03].second == -1)
					{
						vertices.push_back(vertices[index03]);

						vertices[vertices.size() - 1].tangents = glm::vec4(0.0f);
						vertices[vertices.size() - 1].bitangents = glm::vec4(0.0f);

						std::pair<int, int> newHandness;
						newHandness.first = 1;
						newHandness.second = -1;
						handness.push_back(newHandness);

						handness[index03].second = (int)vertices.size() - 1;
					}

					//connect new indice
					indices[3 * i + 2] = handness[index03].second;
					index03 = indices[3 * i + 2];
					vertices[index03].colors.z = 1.0f;
				}


				if (vertices[index01].colors.z == 1.0f || vertices[index02].colors.z == 1.0f || vertices[index03].colors.z == 1.0f)
				{
					vertices[index01].colors.z = 1.0f;
					vertices[index02].colors.z = 1.0f;
					vertices[index03].colors.z = 1.0f;
				}
			}

			if (glm::dot(tan, bit) == 1.0f)
			{
				continue;
			}


		}
		else
		{
			continue;
		}



		glm::vec3 Nor[3];

		Nor[0] = vertices[index01].normals;
		Nor[1] = vertices[index02].normals;
		Nor[2] = vertices[index03].normals;

		glm::vec3 Tan[3];

		Tan[0] = tan;
		Tan[1] = tan;
		Tan[2] = tan;

		glm::vec3 BiTan[3];

		BiTan[0] = bit;
		BiTan[1] = bit;
		BiTan[2] = bit;

		if (Nor[0] != glm::vec3(0.0f))
		{
			nor = Nor[0];

			BiTan[0] = glm::normalize(glm::cross(nor, Tan[0]));
			Tan[0] = glm::normalize(glm::cross(BiTan[0], nor));
		}

		if (Nor[1] != glm::vec3(0.0f))
		{
			nor = Nor[1];
			BiTan[1] = glm::normalize(glm::cross(nor, Tan[1]));
			Tan[1] = glm::normalize(glm::cross(BiTan[1], nor));
		}

		if (Nor[2] != glm::vec3(0.0f))
		{
			nor = Nor[2];
			BiTan[2] = glm::normalize(glm::cross(nor, Tan[2]));
			Tan[2] = glm::normalize(glm::cross(BiTan[2], nor));
		}

		vertices[index01].tangents += glm::vec4(Tan[0], 0.0);
		vertices[index02].tangents += glm::vec4(Tan[1], 0.0);
		vertices[index03].tangents += glm::vec4(Tan[2], 0.0);



		vertices[index01].bitangents += glm::vec4(BiTan[0], 0.0);
		vertices[index02].bitangents += glm::vec4(BiTan[1], 0.0);
		vertices[index03].bitangents += glm::vec4(BiTan[2], 0.0);
	}

	for (size_t i = 0; i < vertices.size(); i++)
	{

		if (glm::length(vertices[i].tangents) == 0.0f)
		{
			if (handness[i].first == 1)
				vertices[i].tangents = glm::vec4(1.0, 0.0, 0.0, 0.0);
			else if (handness[i].first == 0)
				vertices[i].tangents = glm::vec4(-1.0, 0.0, 0.0, 0.0);
			else
			{
				vertices[i].tangents = glm::vec4(1.0, 0.0, 0.0, 0.0);
			}

		}

		if (glm::dot(glm::vec3(vertices[i].normals), glm::vec3(1.0, 0.0, 0.0)) == 1.0f && glm::dot(glm::vec3(vertices[i].normals), glm::vec3(vertices[i].tangents)) > 0.9999f)
		{
			if (handness[i].first == 1)
				vertices[i].tangents = glm::vec4(0.0, 0.0, -1.0, 0.0);
			else
				vertices[i].tangents = glm::vec4(0.0, 0.0, 1.0, 0.0);
		}
		else if (glm::dot(glm::vec3(vertices[i].normals), glm::vec3(-1.0, 0.0, 0.0)) == 1.0f && glm::dot(glm::vec3(vertices[i].normals), glm::vec3(vertices[i].tangents)) > 0.9999f)
		{
			if (handness[i].first == 1)
				vertices[i].tangents = glm::vec4(0.0, 0.0, 1.0, 0.0);
			else
				vertices[i].tangents = glm::vec4(0.0, 0.0, -1.0, 0.0);
		}

		vertices[i].tangents = glm::vec4(normalize(glm::vec3(vertices[i].tangents)), 0.0);

		if (UflipCorrection && vertices[i].colors.z == 1.0f)
			vertices[i].tangents = -vertices[i].tangents;

		vertices[i].bitangents = glm::vec4(normalize(glm::vec3(vertices[i].bitangents)), 0.0);

		glm::vec3 nor = glm::vec3(vertices[i].normals);
		vertices[i].bitangents = glm::vec4(glm::normalize(glm::cross(nor, glm::vec3(vertices[i].tangents))), 0.0);
		vertices[i].tangents = glm::vec4(glm::normalize(glm::cross(glm::vec3(vertices[i].bitangents), nor)), 0.0);
	}
}