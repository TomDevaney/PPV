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

//struct PreDepthPassInput
//{
//	float3 pos : POSITION;
//};

// Per-pixel color data passed through the pixel shader.
struct PS_BasicInput
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float3 uv : TEXCOORD;
	float4 worldPosition : POSITION0;
};

// Simple shader to do vertex processing on the GPU.
PS_BasicInput main(VertexShaderInput input)
{
	PS_BasicInput output;
	float4 pos = float4(input.pos, 1.0f);
	float4 norm = float4(input.normal, 0.0f);

	// Transform the vertex position into projected space.
	pos = mul(pos, model);
	output.worldPosition = pos;
	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos;

	// Pass the color through without modification.
	output.uv = input.uv;

	//pass normal
	norm = mul(norm, model);
	output.normal = (float3)norm;

	return output;
}

//PS_BasicInput PreDepthPass(PreDepthPassInput input)
//{
//	PS_BasicInput output;
//	float4 pos = float4(input.pos, 1.0f);
//
//	// Transform the vertex position into projected space.
//	pos = mul(pos, model);
//	output.worldPosition = pos;
//	pos = mul(pos, view);
//	pos = mul(pos, projection);
//	output.pos = pos;
//
//	return output; //ok to return same pixel structure (I think because it doesn't even make it to shader I'm pretty sure.)
//}

// I want to use this instead of the one above so I don't have to make a new input layout and vertex structure
PS_BasicInput PreDepthPass(VertexShaderInput input)
{
	PS_BasicInput output;
	float4 pos = float4(input.pos, 1.0f);

	// Transform the vertex position into projected space.
	pos = mul(pos, model);
	output.worldPosition = pos;
	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos;

	return output; //ok to return same pixel structure (I think because it doesn't even make it to shader I'm pretty sure.)
}