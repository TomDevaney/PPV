#pragma once
#include <vector>
#include "Bone.h"

class KeyFrame
{
private:
	std::vector<Bone> bones;
	float time;
public:
	//getters
	const Bone* GetBone(unsigned int index) { return &bones[index]; }
	float GetTime() { return time; }
};