#pragma once
#include <DirectXMath.h>
#include "KeyFrame.h"
#include "Bone.h"
#include "TransformNode.h"
//#include "Skeleton.h"

//struct Keyframe
//{
//
//	long mFrameNum;
//	DirectX::XMMATRIX mGlobalTransform;
//	Keyframe* mNext;
//
//	Keyframe() :
//		mNext(nullptr)
//	{}
//};
//
//struct Joint
//{
//	std::string mName;
//	int mParentIndex;
//	DirectX::XMMATRIX mGlobalBindposeInverse;
//	Keyframe* mAnimation;
//	
//
//	Joint() :
//		mAnimation(nullptr)
//	{
//		mGlobalBindposeInverse = DirectX::XMMatrixIdentity();
//		mParentIndex = -1;
//	}
//
//	~Joint()
//	{
//		while (mAnimation)
//		{
//			Keyframe* temp = mAnimation->mNext;
//			delete mAnimation;
//			mAnimation = temp;
//		}
//	}
//};

//struct TomBone
//{
//	std::string name;
//	DirectX::XMMATRIX world;
//	DirectX::XMMATRIX boneOffset; //How far it strayed from original matrix. Used to get new animation state
//	DirectX::XMMATRIX local;
//};

//struct TomKeyFrame
//{
//	std::vector<TomBone> bones;
//};

struct TransformNode
{
	std::string name;
	//unsigned int nameOffset;
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX local;
	//unsigned int parentIndex;
	//unsigned int childIndex;
	//unsigned int siblingIndex;
	TransformNode* parent;
	TransformNode* child;
	TransformNode* sibling;
	bool bDirty;
	unsigned int index = 0;

	//TransformNode()
	//{
	//	nameOffset = -1;
	//	parentIndex = -1;
	//	childIndex = -1;
	//	siblingIndex = -1;
	//}

	void AddChild(TransformNode* tempChild)
	{
		//if (childIndex != -1)
		//{

		//}
		//else
		//{
		//	child
		//}
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

//struct FriendlyIOTransformNode
//{
//	//std::string name;
//	unsigned int nameOffset;
//	DirectX::XMFLOAT4X4 world;
//	DirectX::XMFLOAT4X4 local;
//	int parentIndex;
//	int childIndex;
//	int siblingIndex;
//	//TransformNode* parent;
//	//TransformNode* child;
//	//TransformNode* sibling;
//	bool bDirty;
//
//	FriendlyIOTransformNode()
//	{
//		nameOffset = 0;
//		world = DirectX::XMFLOAT4X4(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
//		local = DirectX::XMFLOAT4X4(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
//		parentIndex = -1;
//		childIndex = -1;
//		siblingIndex = -1;
//		bDirty = false;
//	}
//};

struct TomSkeleton
{
	std::vector<TransformNode*> transforms;
	std::string names = "";
};

struct FriendlyIOSkeleton
{
	std::vector<FriendlyIOTransformNode> transforms;
	std::string names;
};