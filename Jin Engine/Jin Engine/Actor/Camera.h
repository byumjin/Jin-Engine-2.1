#pragma once

#include "../Core/Frustum.h"
#include "../Core/Vulkan.h"
#include "Actor.h"

#define NEAR_PLANE 0.1f
#define FAR_PLANE 1000.0f

class Camera : public Actor
{
public:
	Camera();
	~Camera();

	void setCamera(Vulkan *pVulkanApp, glm::vec3 eyePositionParam, glm::vec3 focusPositionParam, glm::vec3 upVectorParam, float fovYParam, float width, float height, float nearParam, float farParam);
	void updateAspectRatio(float aspectRatio);

	virtual void updateOrbit(float deltaX, float deltaY, float deltaZ);
	virtual void updatePosition(float deltaX, float deltaY, float deltaZ);

	void updateViewMatrix(const glm::mat4 &viewMatParam);
	void updateProjectionMatrix();
	void updateViewProjectionMatrix();

	BoundingBox getViewAABB(BoundingBox &refBox, glm::mat4 &modelViewMat);
	void createCameraBuffer();
	void updateCameraBuffer();
	void shutDown();

	glm::vec3 focusPosition;
	glm::vec3 lookVector;

	glm::mat4 viewMat;
	glm::mat4 projMat;

	glm::mat4 viewProjMat;
	glm::mat4 InvViewProjMat;

	float nearPlane;
	float farPlane;

	float fovY;
	float aspectRatio;
	float focalDistance;

	cameraBuffer cameraBufferInfo;

	VkBuffer uniformCameraBuffer;
	VkDeviceMemory uniformCameraBufferMemory;


	Frustum frustum;

private:
	Vulkan *vulkanApp;

};

