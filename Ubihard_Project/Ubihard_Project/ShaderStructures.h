#pragma once
#include <DirectXMath.h>
#include <algorithm>
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
struct VertexBlendingInfo
{
	unsigned int index;
	double weight;

struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT2 uv;
	XMFLOAT4 blendingIndices;
	XMFLOAT4 blendingWeight;

//	std::vector<VertexBlendingInfo> mVertexBlendingInfos;

	void SortBlendingInfoByWeight()
	{
		std::sort(vertBlendingInfos.begin(), vertBlendingInfos.end());
	}

	bool operator==(const Vertex& rhs) const
	{

		// We only compare the blending info when there is blending info
		if (!(vertBlendingInfos.empty() && rhs.vertBlendingInfos.empty()))
		{
			// Each vertex should only have 4 index-weight blending info pairs
			for (unsigned int i = 0; i < 4; ++i)
			{
				if (vertBlendingInfos[i].index != rhs.vertBlendingInfos[i].index)
				{
					return false;
				}
				if (abs(vertBlendingInfos[i].weight - rhs.vertBlendingInfos[i].weight) > 0.001)
				{
					return false;
				}
			}
		}

		bool r1 = (position.x == rhs.position.x && position.y == rhs.position.y && position.z == rhs.position.z);
		bool r2 = (normal.x == rhs.normal.x && normal.y == rhs.normal.y && normal.z == rhs.normal.z);
		bool r3 = (uv.x == rhs.uv.x && uv.y == rhs.uv.y);

		return r1 && r2 && r3;
	}
};