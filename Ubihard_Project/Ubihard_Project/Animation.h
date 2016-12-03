#pragma once
#include <vector>
#include "AnimationIncludes.h"
#include "KeyFrame.h"

class Animation
{
private:
	AnimType animType;
	std::vector<KeyFrame> keyFrames;
	float totalTime;

public:
	void Init(AnimType type, float time, std::vector<KeyFrame> frames);
	const KeyFrame* GetFrame(unsigned int index) { return &keyFrames[index]; }
	unsigned int GetNumKeyFrames() { return keyFrames.size(); }
};