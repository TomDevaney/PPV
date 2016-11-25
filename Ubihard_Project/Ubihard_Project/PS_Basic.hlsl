struct PS_BasicInput
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
};

float4 main(PS_BasicInput input) : SV_TARGET
{
	float4 resultColor;

	//just basic test
	resultColor = float4(1, 0, 0, 1);

	return resultColor;
}