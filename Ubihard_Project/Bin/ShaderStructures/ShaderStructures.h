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
	bool operator==(const VS_BasicInput& rhs) const
	{

		bool r1 = (position.x == rhs.position.x && position.y == rhs.position.y && position.z == rhs.position.z);
		bool r2 = (normal.x == rhs.normal.x && normal.y == rhs.normal.y && normal.z == rhs.normal.z);
		bool r3 = (uv.x == rhs.uv.x && uv.y == rhs.uv.y);

		return r1 && r2 && r3;
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
// This stores the information of each key frame of each joint
// This is a linked list and each node is a snapshot of the
// global transformation of the joint at a certain frame
struct Keyframe
{

	long mFrameNum;
	XMMATRIX mGlobalTransform;
	Keyframe* mNext;

	Keyframe() :
		mNext(nullptr)
	{}
};
struct Joint
{
	std::string mName;
	int mParentIndex;
	XMMATRIX mGlobalBindposeInverse;
	Keyframe* mAnimation;
	

	Joint() :
		mAnimation(nullptr)
	{
		mGlobalBindposeInverse = XMMatrixIdentity();
		mParentIndex = -1;
	}

	~Joint()
	{
		while (mAnimation)
		{
			Keyframe* temp = mAnimation->mNext;
			delete mAnimation;
			mAnimation = temp;
		}
	}
};

struct Skeleton
{
	std::vector<Joint> mJoints;
};

struct TomBone
{
	std::string name;
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX boneOffset; //How far it strayed from original matrix. Used to get new animation state
	DirectX::XMMATRIX local;
};

struct TomKeyFrame
{
	std::vector<TomBone> bones;
};

struct TransformNode
{
	std::string name;
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX local;
	TransformNode* parent;
	TransformNode* child;
	TransformNode* sibling;
	bool bDirty;

	void AddChild(TransformNode* tempChild)
	{
		if (!child)
		{
			child = tempChild;
		}
		else
		{
			//if there's already a child, give that child a sibling
			child->AddSibling(tempChild);
		}
	}

	void AddSibling(TransformNode* tempSibling)
	{
		if (!sibling)
		{
			sibling = tempSibling;
		}
		else
		{
			sibling->AddSibling(tempSibling);
		}
	}
};

struct TomSkeleton
{
	std::vector<TransformNode*> transforms;
};

struct Vertex
{
	XMFLOAT3 mPosition;
	XMFLOAT3 mNormal;
	XMFLOAT2 mUV;
	XMFLOAT4 blendingIndices;
	XMFLOAT4 blendingWeight;

	bool operator==(const Vertex& rhs) const
	{

		bool r1 = (mPosition.x == rhs.mPosition.x && mPosition.y == rhs.mPosition.y && mPosition.z == rhs.mPosition.z);
		bool r2 = (mNormal.x == rhs.mNormal.x && mNormal.y == rhs.mNormal.y && mNormal.z == rhs.mNormal.z);
		bool r3 = (mUV.x == rhs.mUV.x && mUV.y == rhs.mUV.y);
		bool r4 = (blendingIndices.x == rhs.blendingIndices.x && blendingIndices.y == rhs.blendingIndices.y && blendingIndices.z == rhs.blendingIndices.z);
		bool r5 = (blendingWeight.x == rhs.blendingWeight.x && blendingWeight.y == rhs.blendingWeight.y && blendingWeight.z == rhs.blendingWeight.z);

		return r1 && r2 && r3 && r4 && r5;
	}
};