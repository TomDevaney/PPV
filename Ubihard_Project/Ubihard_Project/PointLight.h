#pragma once
#include "ShaderStructures.h"

class PointLight
{
private:
	PointLightConstantBuffer pointLight;

	const float maxRadius = 5.0f;
	float radiusChange = 1.0f / 60.0f;
public:
	void Create(XMFLOAT4 position, XMFLOAT4 color, float radius);
	void DoRadiusEffect();

	//getters
	PointLightConstantBuffer GetLight() { return pointLight; }
};