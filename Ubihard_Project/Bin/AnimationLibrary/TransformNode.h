#pragma once
#include <DirectXMath.h>

struct FriendlyIOTransformNode
{
	//std::string name;
	unsigned int nameOffset;
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 local;
	int parentIndex;
	int childIndex;
	int siblingIndex;
	bool bDirty;

	//private helper functions
	//void AddSibling(TransformNode2* tempSibling);

	FriendlyIOTransformNode();
	//void AddChild(TransformNode2* tempChild);
};