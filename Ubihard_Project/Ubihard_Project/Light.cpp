#include "Light.h"

void Light::CreateDirLight(XMFLOAT4 normal, XMFLOAT4 color, XMFLOAT4 ambient)
{
	dirLight = new DirectionalLightConstantBuffer();
	pointLight = nullptr;
	spotLight = nullptr;

	dirLight->dirLightNorm = normal;
	dirLight->dirLightColor = color;
	dirLight->ambientLight = ambient;

	lightType = LightTypes::DIRLIGHT;
}

void Light::CreatePointLight(XMFLOAT4 position, XMFLOAT4 color, float radius)
{
	pointLight = new PointLightConstantBuffer();
	dirLight = nullptr;
	spotLight = nullptr;

	pointLight->pointLightPosition = position;
	pointLight->pointLightColor = color;
	pointLight->lightRadius.x = radius;

	lightType = LightTypes::POINTLIGHT;
}

void Light::CreateSpotLight(XMFLOAT4 position, XMFLOAT4 color, float coneRatio, XMFLOAT4 direction)
{
	spotLight = new SpotLightConstantBuffer();
	dirLight = nullptr;
	pointLight = nullptr;

	spotLight->spotLightPosition = position;
	spotLight->spotLightColor = color;
	spotLight->coneRatio.x = coneRatio;
	spotLight->coneDirection = direction;

	lightType = LightTypes::SPOTLIGHT;
}