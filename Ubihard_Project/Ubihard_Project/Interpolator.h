#pragma once
#include <vector>
#include "Animation.h"
#include "Bone.h"
#include "AnimationIncludes.h"

class Interpolator
{
private:
	Animation* animation;
	float curBlendTime;
	unsigned int curFrame;
	float totalBlendTime;
	std::vector<Bone> bones;

public:
	AnimType Update(float time);

	//setters
	void SetAnimation(Animation* anim) { if (animation) delete animation; animation = anim; }
	void SetTotalTime(float time) { totalBlendTime = time; }

	//getters
	Bone* GetCurrentBone(unsigned int index) { return &bones[index]; }
	float GetCurTime() { return curBlendTime; } //I'm pretty sure it wants curTime to see if its under. Might actually want totalTime
	float GetTotalTime() { return totalBlendTime; }
};