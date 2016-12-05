#pragma once
#include <DirectXMath.h>

class TransformNode2
{
private:
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 local;
	//unsigned int parentIndex;
	//unsigned int childIndex;
	//unsigned int siblingIndex;
	//unsigned int nameOffset;
	TransformNode2* parent;
	TransformNode2* child;
	TransformNode2* sibling;
	bool bDirty;

	//private helper functions
	void AddSibling(TransformNode2* tempSibling);

public:
	TransformNode2();
	void AddChild(TransformNode2* tempChild);
};