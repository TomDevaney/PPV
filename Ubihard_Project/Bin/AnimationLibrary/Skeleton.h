#pragma once
#include <vector>
#include <string>
#include "TransformNode.h"

class Skeleton
{
private:
	std::vector<FriendlyIOTransformNode> bones;
	std::string names;
	
public:
	const std::vector<FriendlyIOTransformNode>& GetBones() { return bones; }
	unsigned int GetNumBones() { return (unsigned int)bones.size(); }
	void Init(std::vector<FriendlyIOTransformNode> tempBones, std::string boneNames);
};