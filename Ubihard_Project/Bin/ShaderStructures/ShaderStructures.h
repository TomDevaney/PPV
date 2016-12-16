#pragma once
#define MAXBONES 100
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

struct CameraPositionConstantBuffer
{
	XMFLOAT4 cameraposW;
};

struct VS_BasicInput
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT2 uv;
	bool operator==(const VS_BasicInput& rhs) const
	{

		if (!(uv.x == rhs.uv.x && uv.y == rhs.uv.y)) { return false; }
		if (!(position.x == rhs.position.x && position.y == rhs.position.y && position.z == rhs.position.z)) { return false; }
		if (!(normal.x == rhs.normal.x && normal.y == rhs.normal.y && normal.z == rhs.normal.z)) { return false; }

		return true;
	}
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
		mBlendingWeight(0.0)
	{}

	bool operator < (const VertexBlendingInfo& rhs)
	{
		return (mBlendingWeight > rhs.mBlendingWeight);
	}
};

// Each Control Point in FBX is basically a vertex
// in the physical world. For example, a cube has 8
// vertices(Control Points) in FBX
// Joints are associated with Control Points in FBX
// The mapping is one joint corresponding to 4
// Control Points(Reverse of what is done in a game engine)
// As a result, this struct stores a XMFLOAT3 and a 
// vector of joint indices
struct CtrlPoint
{
	XMFLOAT3 mPosition;
	std::vector<VertexBlendingInfo> mBlendingInfo;

	CtrlPoint()
	{
		mBlendingInfo.reserve(4);
	}
};


struct Vertex
{
	XMFLOAT3 mPosition;
	XMFLOAT3 mNormal;
	XMFLOAT2 mUV;
	XMFLOAT3 mBinormal;
	XMFLOAT4 mTangent;
	XMINT4 mBlendingIndices;
	XMFLOAT4 mBlendingWeight;

	bool operator==(const Vertex& rhs) const
	{
		if (!(mUV.x == rhs.mUV.x && mUV.y == rhs.mUV.y)) { return false; }
		if (!(mPosition.x == rhs.mPosition.x && mPosition.y == rhs.mPosition.y && mPosition.z == rhs.mPosition.z)) { return false; }
		if (!(mNormal.x == rhs.mNormal.x && mNormal.y == rhs.mNormal.y && mNormal.z == rhs.mNormal.z)) { return false; }
		if (!(mBlendingIndices.x == rhs.mBlendingIndices.x && mBlendingIndices.y == rhs.mBlendingIndices.y && mBlendingIndices.z == rhs.mBlendingIndices.z)) { return false; }
		if (!(mBlendingWeight.x == rhs.mBlendingWeight.x && mBlendingWeight.y == rhs.mBlendingWeight.y && mBlendingWeight.z == rhs.mBlendingWeight.z)) { return false; }
		if (!(mBinormal.x == rhs.mBinormal.x && mBinormal.y == rhs.mBinormal.y && mBinormal.z == rhs.mBinormal.z)) { return false; }
		if (!(mTangent.x == rhs.mTangent.x && mTangent.y == rhs.mTangent.y && mTangent.z == rhs.mTangent.z)) { return false; }

		return (true);
	}
};