#pragma once

#include "../Actor/Light.h"

class Sky
{
public:


	Sky()
	{
		sun.updateOrbit(90.0f, 0.0f, 0.0);
	}

	DirectionalLight sun;

private:

};