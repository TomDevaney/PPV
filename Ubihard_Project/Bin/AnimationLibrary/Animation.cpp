#include "Animation.h"

void Animation::Init(AnimType type, float time, std::vector<KeyFrame> frames)
{
	animType = type;
	totalTime = time;
	keyFrames = frames;
}