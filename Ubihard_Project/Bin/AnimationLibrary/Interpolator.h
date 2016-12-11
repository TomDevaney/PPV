#pragma once
#include <vector>
#include "Animation.h"
#include "Bone.h"
#include "AnimationIncludes.h"

class Interpolator
{
private:
	Animation* animation = nullptr;
	float curTime;
	float frameTime;
	unsigned int prevFrame;
	unsigned int curFrame;
	unsigned int nextFrame;
	//float totalBlendTime;
	std::vector<Bone>* bones;
	bool timeBased;
	KeyFrame betweenKeyFrame;

	//private helper functions
	void Interpolate(KeyFrame* previous, KeyFrame* next, float ratio);

public:
	//Constructor
	Interpolator();

	//Destructor
	~Interpolator();

	//Misc
	AnimType Update(float time);

	//setters
	void SetAnimation(Animation* anim) { animation = anim; }
	//void SetTotalTime(float time) { totalBlendTime = time; }
	void SetCurFrame(unsigned int index) { curFrame = index; }
	void SetIsTimeBased(bool toggle) { timeBased = toggle; }

	//getters
	Bone GetCurrentBone(unsigned int index) { return (*bones)[index]; }
	float GetCurTime() { return curTime; } //I'm pretty sure it wants curTime to see if its under. Might actually want totalTime
	//float GetTotalTime() { return totalBlendTime; }
	std::vector<Bone>* GetBones() { return bones; }
};