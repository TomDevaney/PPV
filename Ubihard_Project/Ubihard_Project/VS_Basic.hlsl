// A constant buffer that stores the three basic column-major matrices for composing geometry.
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float3 uv : TEXCOORD;
};

// Per-pixel color data passed through the pixel shader.
struct PS_BasicInput
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float3 uv : TEXCOORD;
};

// Simple shader to do vertex processing on the GPU.
PS_BasicInput main(VertexShaderInput input)
{
	PS_BasicInput output;
	float4 pos = float4(input.pos, 1.0f);

	//bind math

	// Transform the vertex position into projected space.
	pos = mul(pos, model);
	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos;

	// Pass the color through without modification.
	output.uv = input.uv;

	//pass normal
	output.normal = input.normal;

	return output;
}
