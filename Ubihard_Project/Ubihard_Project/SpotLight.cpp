#include "SpotLight.h"

void SpotLight::Create(XMFLOAT4 position, XMFLOAT4 color, float coneRatio, XMFLOAT4 direction)
{
	spotLight.spotLightPosition = position;
	spotLight.spotLightColor = color;
	spotLight.coneRatio.x = coneRatio;
	spotLight.coneDirection = direction;
}