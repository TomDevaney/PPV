#include "TransformNode.h"

TransformNode2::TransformNode2()
{
	parent = nullptr;
	child = nullptr;
	sibling = nullptr;
}

void TransformNode2::AddChild(TransformNode2* tempChild)
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

void TransformNode2::AddSibling(TransformNode2* tempSibling)
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