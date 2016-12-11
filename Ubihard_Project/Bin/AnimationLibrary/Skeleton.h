#pragma once
#include <vector>
#include <string>
#include "TransformNode.h"

class Skeleton
{
private:
	std::vector<FriendlyIOTransformNode> bones;
	std::vector<DirectX::XMFLOAT4X4> inverseBindPoses;
	std::string names;
	
public:
	void Init(std::vector<FriendlyIOTransformNode> tempBones, std::string boneNames, std::vector<DirectX::XMFLOAT4X4> invBindPoses);

	//getters
	const std::vector<FriendlyIOTransformNode>& GetBones() { return bones; }
	unsigned int GetNumBones() { return (unsigned int)bones.size(); }
	DirectX::XMFLOAT4X4 GetInverseBindPose(unsigned int index) { return inverseBindPoses[index]; }
	std::string GetNames() { return names; }
};