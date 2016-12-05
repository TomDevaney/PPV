#pragma once
#include <vector>
#include <string>
#include "TransformNode.h"

class Skeleton
{
private:
	std::vector<TransformNode2> bones;
	std::string names;
	
public:
	const std::vector<TransformNode2>& GetBones() { return bones; }
	unsigned int GetNumBones() { return (unsigned int)bones.size(); }
	void Init(std::vector<TransformNode2> tempBones, std::string boneNames);
};