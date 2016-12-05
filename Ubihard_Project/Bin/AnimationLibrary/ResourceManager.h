#pragma once
#include "Animation.h"
#include "AnimationSet.h"
#include "Skeleton.h"
#include "HashString.h"

#include <map>
#include <string>

//class Animation;
//class BindPose;
//class AnimationSet;

class ResourceManager
{
private:
	std::string resourcesPath = "../Resources/";
	std::map<unsigned int, AnimationSet> animations;
	std::map<unsigned int, Skeleton*> rigs;
	HashString hashString;

public:
	void LoadInSkeleton();
	void LoadInAnimation();

	//getters

};