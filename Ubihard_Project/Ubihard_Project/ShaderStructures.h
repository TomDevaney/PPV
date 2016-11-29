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

struct VertexBlendingInfo
{
	unsigned int mBlendingIndex;
	double mBlendingWeight;

	VertexBlendingInfo() :
		mBlendingIndex(0),
		mBlendingWeight(0.25)
	{}

	bool operator < (const VertexBlendingInfo& rhs)
	{
		return (mBlendingWeight > rhs.mBlendingWeight);
	}
};

struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT2 uv;
	std::vector<VertexBlendingInfo> mVertexBlendingInfos;

	void SortBlendingInfoByWeight()
	{
		std::sort(mVertexBlendingInfos.begin(), mVertexBlendingInfos.end());
	}

	bool operator==(const Vertex& rhs) const
	{

		// We only compare the blending info when there is blending info
		if (!(mVertexBlendingInfos.empty() && rhs.mVertexBlendingInfos.empty()))
		{
			// Each vertex should only have 4 index-weight blending info pairs
			for (unsigned int i = 0; i < 4; ++i)
			{
				if (mVertexBlendingInfos[i].mBlendingIndex != rhs.mVertexBlendingInfos[i].mBlendingIndex)
				{
					return false;
				}
				if (abs(mVertexBlendingInfos[i].mBlendingWeight - rhs.mVertexBlendingInfos[i].mBlendingWeight) > 0.001)
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