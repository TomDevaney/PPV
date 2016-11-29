#include "PointLight.h"

void PointLight::Create(XMFLOAT4 position, XMFLOAT4 color, float radius)
{
	pointLight.pointLightPosition = position;
	pointLight.pointLightColor = color;
	pointLight.lightRadius.x = radius;
}

void PointLight::DoRadiusEffect()
{
	if (pointLight.lightRadius.x > maxRadius)
	{
		pointLight.lightRadius.x = maxRadius;
		radiusChange = -radiusChange;
	}
	else if (pointLight.lightRadius.x < 0.0f)
	{
		pointLight.lightRadius.x = 0.0f;
		radiusChange = -radiusChange;
	}

	pointLight.lightRadius.x += radiusChange;

}