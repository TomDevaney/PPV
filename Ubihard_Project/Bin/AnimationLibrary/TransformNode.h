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
	bool bDirty; //whenever you change the local of this, its children and stuffare dirty. If you get that world, and its dirty, update it.

	//private helper functions
	void AddSibling(TransformNode2* tempSibling);

public:
	TransformNode2();
	void AddChild(TransformNode2* tempChild);
};


// v' = C(B^-1 * v);