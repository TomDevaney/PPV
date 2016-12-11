#pragma once
#include <DirectXMath.h>
#include "KeyFrame.h"
#include "Bone.h"
#include "TransformNode.h"
//#include "Skeleton.h"

struct TransformNode
{
	std::string name;
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX local;
	TransformNode* parent;
	TransformNode* child;
	TransformNode* sibling;
	bool bDirty;
	unsigned int index = 0;

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
	std::vector<DirectX::XMFLOAT4X4> inverseBindPoses;
	std::string names = "";
};

struct FriendlyIOSkeleton
{
	std::vector<FriendlyIOTransformNode> transforms;
	std::vector<DirectX::XMFLOAT4X4> inverseBindPoses;
	std::string names;
};