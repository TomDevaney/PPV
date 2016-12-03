#pragma once
#include "../ShaderStructures/ShaderStructures.h"
#define RADIUSCHANGE 1.0f / 60.0f

class PointLight
{
private:
	PointLightConstantBuffer pointLight;

public:
	void Create(XMFLOAT4 position, XMFLOAT4 color, float radius);
	void DoRadiusEffect(float maxRadius, float& radiusChange);

	//getters
	PointLightConstantBuffer GetLight() { return pointLight; }
};