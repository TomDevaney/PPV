#define NUMOFPOINTLIGHTS 1
//#define NUMOFSPOTLIGHTS 2

struct PS_BasicInput
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
	float4 worldPosition : POSITION0;
};

cbuffer DirectionalLightCB : register(b0)
{
	float4 dirLightNorm;
	float4 dirLightColor;
	float4 ambientLight;
};

cbuffer PointLightCB : register(b1)
{
	float4 pointLightPosition[NUMOFPOINTLIGHTS];
	float4 pointLightColor[NUMOFPOINTLIGHTS];
	float4 lightRadius[NUMOFPOINTLIGHTS]; //treat as float
};

//cbuffer SpotLightCB : register(b2)
//{
//	float4 spotLightPosition[NUMOFSPOTLIGHTS];
//	float4 spotLightColor[NUMOFSPOTLIGHTS];
//	float4 coneRatio[NUMOFSPOTLIGHTS]; //treat as float
//	float4 coneDirection[NUMOFSPOTLIGHTS];
//}

struct PointLight
{
	float4 pointLightPosition;
	float4 pointLightColor;
	float4 lightRadius; //treat as float
};

//StructuredBuffer<PointLight> pointLights : register(u0);

texture2D baseTexture : register(t0);

SamplerState filter : register(s0);

float4 main(PS_BasicInput input) : SV_TARGET
{
	float4 finalColor;
	float3 diffuseColor, dirColor, pointColor, spotColor;

	//initialize point and spot colors
	pointColor = float3(0, 0, 0);
	spotColor = float3(0, 0, 0);

	//get texture color with uvs help
	diffuseColor = (float3)baseTexture.Sample(filter, input.uv);

	//calculate dircolor
	float lightRatio;
	float3 black = { 0.2f, 0.2f, 0.2f };

	lightRatio = saturate(dot((float3)normalize(dirLightNorm), normalize(input.normal)));
	dirColor = (lightRatio + (float3)ambientLight) * (float3)dirLightColor * black;

	//calculate pointlight colors
	//for (int i = 0; i < NUMOFPOINTLIGHTS; ++i)
	{
		float pointRatio;
		float4 pointDir;
		float pointAttenuation;

		pointDir = pointLightPosition[0] - input.worldPosition;
		pointAttenuation = 1 - saturate(length(pointDir) / lightRadius[0].x);
		pointRatio = saturate(dot((float3)normalize(pointDir), normalize(input.normal)));

		pointColor += (float3)pointLightColor[0] * saturate(pointRatio * pointAttenuation) * black;
	}

	//calculate spotlight colors
	//for (int index = 0; index < NUMOFSPOTLIGHTS; ++index)
	//{
	//	float3 spotDir;
	//	float spotRatio;
	//	float spotAttenuation;
	//	float spotSurfaceRatio;
	//	float spotFactor;

	//	spotDir = (float3)normalize(spotLightPosition[index] - input.worldPosition);

	//	spotSurfaceRatio = saturate(dot(-spotDir, (float3)normalize(coneDirection[index])));
	//	spotFactor = (spotSurfaceRatio > coneRatio[index].x) ? 1 : 0;

	//	spotRatio = saturate(dot(spotDir, normalize(input.normal)));

	//	spotColor += saturate(lightRatio /** attenuation*/) * black * (float3)spotLightColor[index] * spotFactor;
	//}

	//calculate final color
	finalColor = float4(saturate((dirColor + pointColor + spotColor) * diffuseColor), 1.0f);

	return finalColor;
}

//float4 main(PS_BasicInput input) : SV_TARGET
//{
//	return input.position;
//}