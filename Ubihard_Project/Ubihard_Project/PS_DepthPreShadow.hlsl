struct PS_BasicInput
{
	float4 pos : SV_POSITION;
	float3 position2D : TEXCOORD0;
};

float4 main(PS_BasicInput input) : SV_TARGET
{
	float4 color;
	float maxDepth;

	maxDepth = 60.0f;
	color = input.position2D.z / maxDepth;

	return float4(1.0f, 0, 0, 0);

	//return input.pos;
}