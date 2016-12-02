#pragma once
#include <DirectXMath.h>

class TransformNode
{
private:
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 local;
	//unsigned int parentIndex;
	//unsigned int childIndex;
	//unsigned int siblingIndex;
	//unsigned int nameOffset;
	TransformNode* parent;
	TransformNode* child;
	TransformNode* sibling;
	bool bDirty;

	//private helper functions
	void AddSibling(TransformNode* tempSibling);

public:
	TransformNode();
	void AddChild(TransformNode* tempChild);
};