#pragma once
#define MAXBONES 4
#include <DirectXMath.h>
#include <vector>
using namespace DirectX;

struct ModelViewProjectionConstantBuffer
{
	XMFLOAT4X4 model;
	XMFLOAT4X4 view;
	XMFLOAT4X4 projection;
};

struct DirectionalLightConstantBuffer
{
	XMFLOAT4 dirLightNorm;
	XMFLOAT4 dirLightColor;
	XMFLOAT4 ambientLight;
};

struct PointLightConstantBuffer
{
	XMFLOAT4 pointLightPosition;
	XMFLOAT4 pointLightColor;
	XMFLOAT4 lightRadius; //treat as a float
};

struct SpotLightConstantBuffer
{
	XMFLOAT4 spotLightPosition;
	XMFLOAT4 spotLightColor;
	XMFLOAT4 coneRatio; //treat as float
	XMFLOAT4 coneDirection;
};

struct BoneOffsetConstantBuffer
{
	XMFLOAT4X4 boneOffsets[MAXBONES];
};

struct VS_BasicInput
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT2 uv;
};

struct PS_BasicInput
{
	XMFLOAT4 position;
	XMFLOAT3 normal;
	XMFLOAT2 uv;
};

struct Bone
{
	XMFLOAT4X4 worldMat;
	XMFLOAT4X4 bindPoseInv;
	Bone()
	{
		XMStoreFloat4x4(&worldMat, XMMatrixIdentity());
		XMStoreFloat4x4(&bindPoseInv, XMMatrixIdentity());
	}
};

struct Skeleton
{
	std::vector<Bone> bones;
};

struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT2 uv;
	XMFLOAT4 blendingIndices;
	XMFLOAT4 blendingWeight;

	bool operator==(const Vertex& rhs) const
	{

		bool r1 = (position.x == rhs.position.x && position.y == rhs.position.y && position.z == rhs.position.z);
		bool r2 = (normal.x == rhs.normal.x && normal.y == rhs.normal.y && normal.z == rhs.normal.z);
		bool r3 = (uv.x == rhs.uv.x && uv.y == rhs.uv.y);
		bool r4 = (blendingIndices.x == rhs.blendingIndices.x && blendingIndices.y == rhs.blendingIndices.y && blendingIndices.z == rhs.blendingIndices.z);
		bool r5 = (blendingWeight.x == rhs.blendingWeight.x && blendingWeight.y == rhs.blendingWeight.y && blendingWeight.z == rhs.blendingWeight.z);

		return r1 && r2 && r3 && r4 && r5;
	}
};