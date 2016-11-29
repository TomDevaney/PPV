#pragma once
#include "ShaderStructures.h"

class PointLight
{
private:
	PointLightConstantBuffer pointLight;

	const float maxRadius = 2.5f;
	float radiusChange = 0.001f;
public:
	void Create(XMFLOAT4 position, XMFLOAT4 color, float radius);
	void DoRadiusEffect();

	//getters
	PointLightConstantBuffer GetLight() { return pointLight; }
};