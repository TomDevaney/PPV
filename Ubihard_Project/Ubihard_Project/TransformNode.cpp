#include "TransformNode.h"

TransformNode::TransformNode()
{
	parent = nullptr;
	child = nullptr;
	sibling = nullptr;
}

void TransformNode::AddChild(TransformNode* tempChild)
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

void TransformNode::AddSibling(TransformNode* tempSibling)
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