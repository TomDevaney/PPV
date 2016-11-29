#pragma once
#include "ShaderStructures.h"
#include "DeviceResources.h"

//I think I need to just create three different light classes

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

	DirectionalLightConstantBuffer* dirLight;
	PointLightConstantBuffer* pointLight;
	SpotLightConstantBuffer* spotLight;

	//Microsoft::WRL::ComPtr<ID3D11Buffer> lightConstantBuffer;
public:
	void CreateDirLight(XMFLOAT4 normal, XMFLOAT4 color, XMFLOAT4 ambient);
	void CreatePointLight(XMFLOAT4 position, XMFLOAT4 color, float radius);
	void CreateSpotLight(XMFLOAT4 position, XMFLOAT4 color, float coneRatio, XMFLOAT4 direction);

	//getters
	DirectionalLightConstantBuffer* GetDirLight() { return dirLight; }
	PointLightConstantBuffer* GetPointLight() { return pointLight; }
	SpotLightConstantBuffer* GetSpotLight() { return spotLight; }

	//ID3D11Buffer* GetLightBuffer() { return lightConstantBuffer.Get(); }
	//translate()
	//affect radius()
	//etc
};