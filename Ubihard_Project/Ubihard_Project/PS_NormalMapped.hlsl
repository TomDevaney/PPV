#define NUMOFPOINTLIGHTS 1

struct PS_BasicInput
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float3 uv : TEXCOORD;
	float3 binormal : BINORMAL;
	float3 tangent : TANGENT;
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


struct PointLight
{
	float4 pointLightPosition;
	float4 pointLightColor;
	float4 lightRadius; //treat as float
};


texture2D baseTexture : register(t0);
texture2D normalMap : register(t1);

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

	//create new normals based on normals
	float3 bumpNormal = input.normal;

	float4 bumpMap = normalMap.Sample(filter, input.uv);
	bumpMap = (bumpMap * 2.0f) - 1.0f;
	bumpNormal = (bumpMap.x * input.tangent) + (bumpMap.y * input.binormal) + (bumpMap.z * input.normal);
	bumpNormal = normalize(bumpNormal);


	//calculate dircolor
	float lightRatio;
	float3 black = { 0.2f, 0.2f, 0.2f };

	lightRatio = saturate(dot((float3)normalize(dirLightNorm), normalize(bumpNormal)));
	dirColor = (lightRatio + (float3)ambientLight) * (float3)dirLightColor * black;

	float pointRatio;
	float4 pointDir;
	float pointAttenuation;

	pointDir = pointLightPosition[0] - input.worldPosition;
	pointAttenuation = 1 - saturate(length(pointDir) / lightRadius[0].x);
	pointRatio = saturate(dot((float3)normalize(pointDir), normalize(bumpNormal)));

	pointColor += (float3)pointLightColor[0] * saturate(pointRatio * pointAttenuation) * black;

	//calculate final color
	finalColor = float4(saturate((dirColor + pointColor + spotColor) * diffuseColor), 1.0f);

	return finalColor;
}