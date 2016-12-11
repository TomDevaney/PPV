#include "TransformNode.h"

FriendlyIOTransformNode::FriendlyIOTransformNode()
{
	//nameOffset = 0;
	world = DirectX::XMFLOAT4X4(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	local = DirectX::XMFLOAT4X4(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	parentIndex = -1;
	childIndex = -1;
	siblingIndex = -1;
	bDirty = false;
}

//void TransformNode2::AddChild(TransformNode2* tempChild)
//{
//	if (!child)
//	{
//		child = tempChild;
//	}
//	else
//	{
//		//if there's already a child, give that child a sibling
//		child->AddSibling(tempChild);
//	}
//}
//
//void TransformNode2::AddSibling(TransformNode2* tempSibling)
//{
//	if (!sibling)
//	{
//		sibling = tempSibling;
//	}
//	else
//	{
//		sibling->AddSibling(tempSibling);
//	}
//}