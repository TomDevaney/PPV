#include "Skeleton.h"

void Skeleton::Init(std::vector<FriendlyIOTransformNode> tempBones, std::string boneNames)
{
	bones = tempBones;
	names = boneNames;
}