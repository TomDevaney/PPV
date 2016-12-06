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
	std::wstring resourcesPath = L"../Resources";
	std::map<unsigned int, AnimationSet> animationSets;
	//std::map<unsigned int, Skeleton*> rigs;
	HashString* hashString;

	unsigned int animationSetIndex;

	//private helper functions
	Skeleton LoadInSkeleton(std::wstring path);
	Animation LoadInAnimation(std::wstring path);
public:
	ResourceManager();
	~ResourceManager();

	void LoadInAnimationSet();

	//getters
	const AnimationSet& GetAnimationSet(std::string animation) { return animationSets[hashString->GetKey(animation)]; }
};