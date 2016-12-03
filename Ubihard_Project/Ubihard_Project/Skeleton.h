#pragma once
#include <vector>
#include <string>
#include "TransformNode.h"

class Skeleton
{
private:
	std::vector<TransformNode> bones;
	std::string names;
	
public:
	const std::vector<TransformNode>& GetBones() { return bones; }
	unsigned int GetNumBones() { return bones.size(); }
	void Init(std::vector<TransformNode> tempBones, std::string boneNames);
};