#pragma once
#include "Animation.h"
#include "AnimationSet.h"
#include "Skeleton.h"

#include <map>

//class Animation;
//class BindPose;
//class AnimationSet;

class ResourceManager
{
private:
	std::map<unsigned int, AnimationSet> animations;
	std::map<unsigned int, Skeleton*> rigs;

public:
};