#pragma once
#include <vector>
#include <DirectXMath.h>
#include "AnimationSet.h"
#include "AnimationIncludes.h"
#include "Interpolator.h"
#include "HashString.h"
#include "ResourceManager.h"

class Blender
{
private:
	const AnimationSet* animationSet;
	BlendInfo blendInfo;
	std::vector<DirectX::XMFLOAT4X4> inverseBindPoses;
	Interpolator* curAnim;
	Interpolator* nextAnim;
	HashString* hashString;
	ResourceManager* resourceManager;

public:
	Blender();

	void Update(float time);

	//getters


	//setters
	void SetAnimationState(std::string key) { animationSet = &resourceManager->GetAnimationSet(key); }
};