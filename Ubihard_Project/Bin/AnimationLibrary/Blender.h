#pragma once
#include <vector>
#include <DirectXMath.h>
#include "AnimationSet.h"
#include "AnimationIncludes.h"
#include "Interpolator.h"
#include "HashString.h"

class Blender
{
private:
	AnimationSet* animationSet;
	BlendInfo blendInfo;
	std::vector<DirectX::XMFLOAT4X4> inverseBindPoses;
	Interpolator* curAnim;
	Interpolator* nextAnim;

public:
	void Update(float time);

	//getters


	//setters
	//void SetAnimState(std::string key) {return 
};