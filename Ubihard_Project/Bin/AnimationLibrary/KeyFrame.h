#pragma once
#include <vector>
#include "Bone.h"

class KeyFrame
{
private:
	std::vector<Bone> bones;
	float time;

public:
	KeyFrame();

	//void Init(std::vector<Bone> bons, float t);
	void InsertBone(Bone bone) { bones.push_back(bone); }
	void ClearBones() { bones.clear(); }

	//getters
	Bone GetBone(unsigned int index) { return bones[index]; }
	const std::vector<Bone>& GetBones() { return bones; }
	float GetTime() { return time; }

	//setters
	void SetTime(float t) { time = t; }
	void SetBones(std::vector<Bone> bs) { bones = bs; }
};