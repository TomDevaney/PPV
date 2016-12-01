#pragma once
#include "Animation.h"
#include "AnimationSet.h"
#include "BindPose.h"

#include <map>

//class Animation;
//class BindPose;
//class AnimationSet;

class ResourceManager
{
private:
	std::map<unsigned int, AnimationSet> animations;
	std::map<unsigned int, BindPose*> rigs;

public:
	
};