#define MAXBONES 100

// A constant buffer that stores the three basic column-major matrices for composing geometry.
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

//constant buffer for bone offsets
cbuffer BoneOffsetConstantBuffer : register(b1)
{
	matrix boneOffsets[MAXBONES];
};

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float3 uv : TEXCOORD;
	float3 binormal : BINORMAL;
	float4 tangent : TANGENT;
	int4 blendingIndex : BLENDINDICES;
	float4 blendingWeight : BLENDWEIGHT;
};

// Per-pixel color data passed through the pixel shader.
struct PS_BasicInput
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float3 uv : TEXCOORD;
	float3 binormal : BINORMAL;
	float3 tangent : TANGENT;
	float4 worldPosition : POSITION0;
};

// Simple shader to do vertex processing on the GPU.
PS_BasicInput main(VertexShaderInput input)
{
	PS_BasicInput output;
	float4 pos = float4(input.pos, 1.0f);
	//bind math
	pos = mul(float4(input.pos, 1.0f), boneOffsets[input.blendingIndex.x]) * input.blendingWeight.x;
	pos += mul(float4(input.pos, 1.0f), boneOffsets[input.blendingIndex.y]) * input.blendingWeight.y;
	pos += mul(float4(input.pos, 1.0f), boneOffsets[input.blendingIndex.z]) * input.blendingWeight.z;
	pos += mul(float4(input.pos, 1.0f), boneOffsets[input.blendingIndex.w]) * input.blendingWeight.w;

	// Transform the vertex position into projected space.
	pos = mul(pos, model);
	output.worldPosition = pos;
	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos;

	// Pass the color through without modification.
	output.uv = input.uv;

	//pass normal
	output.normal = mul(float4(input.normal, 0.0f), boneOffsets[input.blendingIndex.x]).xyz * input.blendingWeight.x;
	output.normal += mul(float4(input.normal, 0.0f), boneOffsets[input.blendingIndex.y]).xyz * input.blendingWeight.y;
	output.normal += mul(float4(input.normal, 0.0f), boneOffsets[input.blendingIndex.z]).xyz * input.blendingWeight.z;
	output.normal += mul(float4(input.normal, 0.0f), boneOffsets[input.blendingIndex.w]).xyz * input.blendingWeight.w;

	output.normal = mul(float4(output.normal, 0), model).xyz;

	// Calculate the tangent vector against the world matrix only and then normalize the final value.
	output.tangent  = mul(float4(input.tangent.xyz, 0.0f), boneOffsets[input.blendingIndex.x]).xyz * input.blendingWeight.x;
	output.tangent += mul(float4(input.tangent.xyz, 0.0f), boneOffsets[input.blendingIndex.y]).xyz * input.blendingWeight.y;
	output.tangent += mul(float4(input.tangent.xyz, 0.0f), boneOffsets[input.blendingIndex.z]).xyz * input.blendingWeight.z;
	output.tangent += mul(float4(input.tangent.xyz, 0.0f), boneOffsets[input.blendingIndex.w]).xyz * input.blendingWeight.w;

	output.tangent = mul(float4(output.tangent, 0), model).xyz;
	output.tangent = normalize(output.tangent);

	// Calculate the binormal vector against the world matrix only and then normalize the final value.
	output.binormal = cross(output.normal, output.tangent) * input.tangent.w;
	output.binormal = normalize(output.binormal);

	return output;
}
