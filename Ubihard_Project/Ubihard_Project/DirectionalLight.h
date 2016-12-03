#pragma once
#include "../ShaderStructures/ShaderStructures.h"

class DirectionalLight
{
private:
	DirectionalLightConstantBuffer dirLight;

public:
	void Create(XMFLOAT4 normal, XMFLOAT4 color, XMFLOAT4 ambient);

	DirectionalLightConstantBuffer GetDirLight() { return dirLight; }
};