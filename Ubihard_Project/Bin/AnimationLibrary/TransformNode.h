#pragma once
#include <DirectXMath.h>

struct FriendlyIOTransformNode
{
	//DirectX::XMFLOAT4X4 world;
	//DirectX::XMFLOAT4X4 local;
	////unsigned int parentIndex;
	////unsigned int childIndex;
	////unsigned int siblingIndex;
	////unsigned int nameOffset;
	//TransformNode2* parent;
	//TransformNode2* child;
	//TransformNode2* sibling;
	//bool bDirty; //whenever you change the local of this, its children and stuffare dirty. If you get that world, and its dirty, update it.

				 //std::string name;
	unsigned int nameOffset;
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 local;
	int parentIndex;
	int childIndex;
	int siblingIndex;
	//TransformNode* parent;
	//TransformNode* child;
	//TransformNode* sibling;
	bool bDirty;

	//private helper functions
	//void AddSibling(TransformNode2* tempSibling);

	FriendlyIOTransformNode();
	//void AddChild(TransformNode2* tempChild);
};


// v' = C(B^-1 * v);