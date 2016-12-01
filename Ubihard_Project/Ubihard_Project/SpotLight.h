#pragma once
#include "../ShaderStructures/ShaderStructures.h"

class SpotLight
{
private:
	SpotLightConstantBuffer spotLight;

public:
	void Create(XMFLOAT4 position, XMFLOAT4 color, float coneRatio, XMFLOAT4 direction);
};
