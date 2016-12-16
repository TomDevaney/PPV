#define NUMOFPOINTLIGHTS 1

struct PS_BasicInput
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float3 uv : TEXCOORD;
	float3 binormal : BINORMAL;
	float3 tangent : TANGENT;
	float4 worldPosition : POSITION0;
	float4 lightViewPosition : POSITION1;
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
texture2D shadowMap : register(t3);

SamplerState filter : register(s0);
SamplerState filterClamp : register(s1);

float4 main(PS_BasicInput input) : SV_TARGET
{
	float4 finalColor;
	float3 diffuseColor, dirColor, pointColor, spotColor;

	//Calculate the Tangent-Space matrix. This may need tweaking
	float3x3 TBNMatrix = float3x3(input.binormal, input.tangent, input.normal);

	float3 ViewVector = normalize(cameraposW.xyz - input.worldPosition.xyz);

	//initialize point and spot colors
	pointColor = float3(0, 0, 0);
	spotColor = float3(0, 0, 0);

	//get texture color with uvs help
	diffuseColor = baseTexture.Sample(filter, input.uv.xy).xyz;
	float3 specMap = specularMap.Sample(filter, input.uv.xy).xyz;

	//create new normals based on normals
	float3 bumpNormal = input.normal;

	float4 bumpMap = normalMap.Sample(filter, input.uv.xy);
	bumpMap = (bumpMap * 2.0f) - 1.0f;



	//bumpNormal = (bumpMap.x * input.tangent) + (bumpMap.y * input.binormal) + (bumpMap.z * input.normal);
	bumpNormal = normalize(mul(bumpMap.xyz, TBNMatrix));


	//calculate dircolor
	float lightRatio;
	float3 black = { 0.2f, 0.2f, 0.2f };
	float3 dir_dir = normalize(dirLightNorm).xyz;


	lightRatio = saturate(dot(dir_dir, normalize(bumpNormal)));
	dirColor = (lightRatio + ambientLight.xyz) * dirLightColor.xyz * black;

	//Specular directional
	float3 dirRefelection = normalize(reflect(dir_dir, bumpNormal));
	float dirRdotV = max(0, dot(dirRefelection, ViewVector));
	float specDirScale = pow(dirRdotV, 32);
	float3 dirSpecColor = saturate(specMap *  specDirScale * dirColor);

	float pointRatio;
	float4 pointDir;
	float pointAttenuation;

	pointDir = pointLightPosition[0] - input.worldPosition;
	//pointDir.xyz = mul(pointDir.xyz, TBNMatrix);
	pointAttenuation = 1 - saturate(length(pointDir.xyz) / lightRadius[0].x);
	pointRatio = saturate(dot(normalize(pointDir.xyz), normalize(bumpNormal)));

	pointColor += pointLightColor[0].xyz * saturate(pointRatio * pointAttenuation) * black;

	//Specular point
	float3 pointRefelection = normalize(reflect(-pointDir.xyz, bumpNormal));
	float pointRdotV = max(0, dot(pointRefelection, ViewVector));
	float specPointScale = pow(pointRdotV, 32);
	float3 pointSpecColor = saturate(specMap *  specPointScale * pointColor);

	//Calculate shadow factor
	float shadowFactor, ourDepth;
	float2 texCoords;

	ourDepth = input.lightViewPosition.z / input.lightViewPosition.w;
	float2 tomsCoords;

	tomsCoords.x = input.lightViewPosition.x / input.lightViewPosition.w / 2.0 + 0.5f;
	tomsCoords.y = -input.lightViewPosition.y / input.lightViewPosition.w / 2.0 + 0.5f;

	texCoords = (input.lightViewPosition.xy / input.lightViewPosition.w + 1) * 0.5f;

	//tomsCoords = float2(0.5f, 0.5f);

	float sampleDepth = shadowMap.Sample(filter, tomsCoords).x;

	shadowFactor = (ourDepth <= sampleDepth);

	//calculate final color
	finalColor = float4(saturate((dirColor + pointColor + spotColor) * diffuseColor), 1.0f);
	finalColor.xyz = saturate(finalColor.xyz + pointSpecColor + dirSpecColor);

	finalColor *= (ambientLight + shadowFactor);

	//return float4(input.normal, 1);

	return finalColor;
}