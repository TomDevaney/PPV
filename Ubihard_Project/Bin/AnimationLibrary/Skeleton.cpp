#include "Skeleton.h"

void Skeleton::Init(std::vector<TransformNode2> tempBones, std::string boneNames)
{
	bones = tempBones;
	names = boneNames;
}