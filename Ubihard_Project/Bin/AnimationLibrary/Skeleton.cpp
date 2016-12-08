#include "Skeleton.h"

void Skeleton::Init(std::vector<FriendlyIOTransformNode> tempBones, std::string boneNames, std::vector<DirectX::XMFLOAT4X4> invBindPoses)
{
	bones = tempBones;
	names = boneNames;
	inverseBindPoses = invBindPoses;
}