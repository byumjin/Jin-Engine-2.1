#pragma once

#include "Common.h"

class Frustum
{
public:
	enum side { LEFT = 0, RIGHT = 1, TOP = 2, BOTTOM = 3, BACK = 4, FRONT = 5 };
	
	FrustumInfo frustumInfo;

	//std::array<glm::vec4, 6> planes;

	void update(glm::mat4 matrix)
	{
		frustumInfo.planes[LEFT].x = matrix[0].w + matrix[0].x;
		frustumInfo.planes[LEFT].y = matrix[1].w + matrix[1].x;
		frustumInfo.planes[LEFT].z = matrix[2].w + matrix[2].x;
		frustumInfo.planes[LEFT].w = matrix[3].w + matrix[3].x;

		frustumInfo.planes[RIGHT].x = matrix[0].w - matrix[0].x;
		frustumInfo.planes[RIGHT].y = matrix[1].w - matrix[1].x;
		frustumInfo.planes[RIGHT].z = matrix[2].w - matrix[2].x;
		frustumInfo.planes[RIGHT].w = matrix[3].w - matrix[3].x;

		frustumInfo.planes[TOP].x = matrix[0].w - matrix[0].y;
		frustumInfo.planes[TOP].y = matrix[1].w - matrix[1].y;
		frustumInfo.planes[TOP].z = matrix[2].w - matrix[2].y;
		frustumInfo.planes[TOP].w = matrix[3].w - matrix[3].y;

		frustumInfo.planes[BOTTOM].x = matrix[0].w + matrix[0].y;
		frustumInfo.planes[BOTTOM].y = matrix[1].w + matrix[1].y;
		frustumInfo.planes[BOTTOM].z = matrix[2].w + matrix[2].y;
		frustumInfo.planes[BOTTOM].w = matrix[3].w + matrix[3].y;

		frustumInfo.planes[BACK].x = matrix[0].w + matrix[0].z;
		frustumInfo.planes[BACK].y = matrix[1].w + matrix[1].z;
		frustumInfo.planes[BACK].z = matrix[2].w + matrix[2].z;
		frustumInfo.planes[BACK].w = matrix[3].w + matrix[3].z;

		frustumInfo.planes[FRONT].x = matrix[0].w - matrix[0].z;
		frustumInfo.planes[FRONT].y = matrix[1].w - matrix[1].z;
		frustumInfo.planes[FRONT].z = matrix[2].w - matrix[2].z;
		frustumInfo.planes[FRONT].w = matrix[3].w - matrix[3].z;

		for (auto i = 0; i < 6; i++)
		{
			float length = sqrtf(frustumInfo.planes[i].x * frustumInfo.planes[i].x + frustumInfo.planes[i].y * frustumInfo.planes[i].y + frustumInfo.planes[i].z * frustumInfo.planes[i].z);
			frustumInfo.planes[i] /= length;
		}
	}

	bool checkBox(BoundingBox &viewAABB)
	{
		bool result = true;

		for (auto i = 0; i < 6; i++)
		{
			glm::vec4 positivePoint = viewAABB.minPt;
			glm::vec4 negativePoint = viewAABB.maxPt;

			if (frustumInfo.planes[i].x >= 0.0)
			{
				positivePoint.x = viewAABB.maxPt.x;
				negativePoint.x = viewAABB.minPt.x;
			}
			if (frustumInfo.planes[i].y >= 0.0)
			{
				positivePoint.y = viewAABB.maxPt.y;
				negativePoint.y = viewAABB.minPt.y;
			}
			if (frustumInfo.planes[i].z >= 0.0)
			{
				positivePoint.z = viewAABB.maxPt.z;
				negativePoint.z = viewAABB.minPt.z;
			}

			// is the positive vertex outside?			
			if (glm::dot(glm::vec3(positivePoint), glm::vec3(frustumInfo.planes[i])) + frustumInfo.planes[i].w < 0.0)
			{
				return false;
			}
		}
		
		return result;
	}

	bool checkSphere(glm::vec3 pos, float radius)
	{
		for (auto i = 0; i < 6; i++)
		{
			if ((frustumInfo.planes[i].x * pos.x) + (frustumInfo.planes[i].y * pos.y) + (frustumInfo.planes[i].z * pos.z) + frustumInfo.planes[i].w <= -radius)
			{
				return false;
			}
		}
		return true;
	}
};
