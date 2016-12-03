#include "Skeleton.h"

void Skeleton::Init(std::vector<TransformNode> tempBones, std::string boneNames)
{
	bones = tempBones;
	names = boneNames;
}