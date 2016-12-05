#pragma once
#include <vector>
#include "Animation.h"
#include "Skeleton.h"
#include "AnimationIncludes.h"

class AnimationSet
{
private:
	std::vector<Animation> animations;
	const Skeleton* skeleton;
	std::vector<std::vector<BlendInfo>> blendInfos;
	unsigned int defaultAnimation;

public:
	bool AddAnimation(FILE* file);
	bool AddSkeleton(const Skeleton* skele);

	//getters
	const Animation* GetAnimation(unsigned int index) { return &animations[index]; }
	const BlendInfo* GetBlendInfo(unsigned int animationFrom, unsigned int animationTo) { return &blendInfos[animationFrom][animationTo]; } // TODO: good chance this is wrong
	const Animation* GetDefaultAnimation() { return &animations[defaultAnimation]; }
};