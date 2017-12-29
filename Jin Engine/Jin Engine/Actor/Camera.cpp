#include "Camera.h"

Camera::Camera(): focusPosition(glm::vec3(0.0f, 3.0f, 0.0f)), focalDistance(10.0f)
{
	position = glm::vec3(0.0f, 3.0f, 5.0f);
	upVector = glm::vec3(0.0f, 1.0f, 0.0f);
}

Camera::~Camera()
{
	
}

void Camera::setCamera(Vulkan *pVulkanApp, glm::vec3 eyePositionParam, glm::vec3 focusPositionParam, glm::vec3 upVectorParam, float fovYParam, float width, float height, float nearParam, float farParam)
{
	vulkanApp = pVulkanApp;

	position = eyePositionParam;
	focusPosition = focusPositionParam;
	upVector = upVectorParam;
	fovY = fovYParam;

	nearPlane = nearParam;
	farPlane = farParam;

	aspectRatio = width / height;

	//lookVector = glm::normalize(focusPosition - position);

	updateViewMatrix(glm::lookAt(position, focusPosition, upVector));
	//updateProjectionMatrix();

	createCameraBuffer();

	updateAspectRatio(aspectRatio);
}

void Camera::updateViewMatrix(const glm::mat4 &viewMatParam)
{
	viewMat = viewMatParam;
	//lookVector = glm::vec3(viewMat[2].x, viewMat[2].y, viewMat[2].z);
	lookVector = -glm::vec3(modelMat[2].x, modelMat[2].y, modelMat[2].z);
	focusPosition = lookVector + position;
}

void Camera::updateProjectionMatrix()
{
	projMat = glm::perspective(glm::radians(fovY), aspectRatio, nearPlane, farPlane);
	frustum.update(projMat);
	projMat[1][1] *= -1.0;

	
}

void Camera::updateViewProjectionMatrix()
{
	viewProjMat = projMat * viewMat;
	InvViewProjMat = glm::inverse(viewProjMat);
}

void Camera::updateAspectRatio(float aspectRatioParam)
{
	aspectRatio = aspectRatioParam;

	updateProjectionMatrix();
	updateViewProjectionMatrix();

	updateCameraBuffer();
}

void Camera::updateOrbit(float deltaX, float deltaY, float deltaZ)
{
	Actor::updateOrbit(deltaX, deltaY, deltaZ);

	updateViewMatrix(glm::inverse(modelMat));
	updateViewProjectionMatrix();

	updateCameraBuffer();
}

void Camera::updatePosition(float deltaX, float deltaY, float deltaZ)
{
	Actor::updatePosition(deltaX, deltaY, deltaZ);

	updateViewMatrix(glm::inverse(modelMat));
	updateViewProjectionMatrix();

	updateCameraBuffer();
}

BoundingBox Camera::getViewAABB(BoundingBox &refBox, glm::mat4 &modelViewMat)
{
	BoundingBox viewBox;

	viewBox.maxPt = viewBox.minPt = modelViewMat * glm::vec4(glm::vec3(refBox.corners[0]), 1.0);

	for (size_t i = 1; i < 8; ++i)
	{
		glm::vec4 viewCorner = modelViewMat * glm::vec4(glm::vec3(refBox.corners[i]), 1.0);
		viewBox.maxPt = glm::max(viewCorner, viewBox.maxPt);
		viewBox.minPt = glm::min(viewCorner, viewBox.minPt);
	}

	viewBox.Center = (viewBox.maxPt + viewBox.minPt) * 0.5f;
	viewBox.Extents = glm::abs(viewBox.maxPt - viewBox.minPt) * 0.5f;

	return viewBox;
}

void Camera::createCameraBuffer()
{
	vulkanApp->createBuffer(sizeof(cameraBuffer), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformCameraBuffer, uniformCameraBufferMemory);

	updateCameraBuffer();
}

void Camera::updateCameraBuffer()
{
	cameraBufferInfo.viewMat = viewMat;
	cameraBufferInfo.projMat = projMat;
	cameraBufferInfo.viewProjMat = viewProjMat;
	cameraBufferInfo.InvViewProjMat = InvViewProjMat;

	cameraBufferInfo.cameraWorldPos = glm::vec4(position, 1.0);

	vulkanApp->updateBuffer(&cameraBufferInfo, uniformCameraBufferMemory, sizeof(cameraBuffer));
}

void Camera::shutDown()
{
	vkDestroyBuffer(vulkanApp->getDevice(), uniformCameraBuffer, nullptr);
	vkFreeMemory(vulkanApp->getDevice(), uniformCameraBufferMemory, nullptr);
}