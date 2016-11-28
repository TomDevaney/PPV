#pragma once
#include "ShaderStructures.h"

class Light
{
private:
	enum LightTypes
	{
		DIRLIGHT = 0,
		POINTLIGHT,
		SPOTLIGHT,
	};

	LightTypes lightType;


public:

};