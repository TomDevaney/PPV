#include "DirectionalLight.h"

void DirectionalLight::Create(XMFLOAT4 normal, XMFLOAT4 color, XMFLOAT4 ambient)
{
	dirLight.dirLightNorm = normal;
	dirLight.dirLightColor = color;
	dirLight.ambientLight = ambient;
}