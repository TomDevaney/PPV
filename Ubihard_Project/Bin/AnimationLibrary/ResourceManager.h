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
	std::map<unsigned int, AnimationSet> animationSets;
	//std::map<unsigned int, Skeleton*> rigs;
	HashString hashString;

	void LoadInSkeleton();
	void LoadInAnimation();
public:
	void LoadInAnimationSet();

	//getters
	const AnimationSet& GetAnimationSet(std::string animation) { return animationSets[hashString.GetKey(animation)]; }
};