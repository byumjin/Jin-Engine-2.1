#pragma once

#include "../core/Common.h"

class Actor
{
public:
	Actor();
	~Actor();

	void update();
	virtual void updateOrbit(float deltaX, float deltaY, float deltaZ);
	virtual void updatePosition(float deltaX, float deltaY, float deltaZ);

	glm::mat4 modelMat;
	glm::mat4 InvTransposeMat;
	glm::mat4 rotation;

	glm::vec3 position;
	glm::vec3 scale;
	glm::vec3 upVector;

	float radious, theta, phi;

protected:

	std::string actorName;
};
