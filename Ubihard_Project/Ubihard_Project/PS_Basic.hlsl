struct PS_BasicInput
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
};

texture2D baseTexture : register(t0);
SamplerState filter : register(s0);

float4 main(PS_BasicInput input) : SV_TARGET
{
	float4 resultColor;

	resultColor = baseTexture.Sample(filter, input.uv);

	return resultColor;
}