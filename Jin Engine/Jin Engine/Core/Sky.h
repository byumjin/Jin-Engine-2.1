#pragma once

#include "../Actor/Light.h"
#include "../Asset/Texture.h"

class Sky
{
public:
	
	Sky()
	{
		sun.updateOrbit(90.0f, 0.0f, 0.0);

		lowFreqTexture = new Texture;
		highFreqTexture = new Texture;
	}

	void shutDown()
	{
		lowFreqTexture->shutDown();
		highFreqTexture->shutDown();
	}

	DirectionalLight sun;

	Texture* lowFreqTexture;
	Texture* highFreqTexture;

private:

};