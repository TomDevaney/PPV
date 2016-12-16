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

cbuffer CameraPosition : register(b2)
{
	float4 cameraposW;
}

struct PointLight
{
	float4 pointLightPosition;
	float4 pointLightColor;
	float4 lightRadius; //treat as float
};


texture2D baseTexture : register(t0);
texture2D normalMap : register(t1);
texture2D specularMap : register(t2);

SamplerState filter : register(s0);

float4 main(PS_BasicInput input) : SV_TARGET
{
	float4 finalColor;
	float3 diffuseColor, dirColor, pointColor, spotColor;

	//Calculate the Tangent-Space matrix. This may need tweaking
	//float3x3 TBNMatrix = float3x3(input.tangent.x, input.binormal.x, input.normal.x,
	//	input.tangent.y, input.binormal.y, input.normal.y,
	//	input.tangent.z, input.binormal.z, input.normal.z);
		
		
	float3x3 TBNMatrix = float3x3(input.tangent, input.binormal, input.normal);

	float3 ViewVector = normalize(cameraposW.xyz - input.worldPosition.xyz);

	//initialize point and spot colors
	pointColor = float3(0, 0, 0);
	spotColor = float3(0, 0, 0);

	//get texture color with uvs help
	diffuseColor = baseTexture.Sample(filter, input.uv.xy).xyz;
	float3 specMap = specularMap.Sample(filter, input.uv.xy).xyz;

	//create new normals based on normals

	float3 bumpMap = normalMap.Sample(filter, input.uv).xyz;
	bumpMap = (bumpMap * 2.0f) - 1.0f;



	float3 bumpNormal = normalize(mul(bumpMap.xyz, TBNMatrix));
	bumpNormal = bumpNormal * 0.5f + 0.5f;

	//calculate dircolor
	float lightRatio;
	float3 black = { 0.2f, 0.2f, 0.2f };
	float3 dir_dir = normalize(dirLightNorm).xyz;


	lightRatio = saturate(dot(dir_dir, bumpNormal));
	dirColor = (lightRatio + ambientLight.xyz) * dirLightColor.xyz * black;

	//Specular directional
	float3 dirRefelection = normalize(reflect(dir_dir, bumpNormal));
	float dirRdotV = max(0, dot(dirRefelection, ViewVector));
	float specDirScale = pow(dirRdotV, 32);
	float3 dirSpecColor = saturate( specMap *  specDirScale * dirColor);

	float pointRatio;
	float4 pointDir;
	float pointAttenuation;

	pointDir = pointLightPosition[0] - input.worldPosition;
	//pointDir.xyz = mul(pointDir.xyz, TBNMatrix);
	pointAttenuation = 1 - saturate(length(pointDir.xyz) / lightRadius[0].x);
	pointRatio = saturate(dot(normalize(pointDir.xyz), bumpNormal));

	pointColor += pointLightColor[0].xyz * saturate(pointRatio * pointAttenuation) * black;

	//Specular point
	float3 pointRefelection = normalize(reflect(-pointDir.xyz, bumpNormal));
	float pointRdotV = max(0, dot(pointRefelection, ViewVector));
	float specPointScale = pow(pointRdotV, 32);
	float3 pointSpecColor = saturate( specMap * specPointScale * pointColor);


	//calculate final color
	finalColor = float4(saturate((dirColor + pointColor + spotColor) * diffuseColor), 1.0f);
	finalColor.xyz = saturate(finalColor.xyz + pointSpecColor + dirSpecColor);

	//return float4(bumpNormal, 1);

	return finalColor;
}