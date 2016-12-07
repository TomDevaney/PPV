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
	AnimationSet* animationSet;
	BlendInfo blendInfo;
	std::vector<DirectX::XMFLOAT4X4> boneOffsets;
	std::vector<DirectX::XMFLOAT4X4> bonesWorlds;
	Interpolator* curAnim;
	Interpolator* nextAnim;
	HashString* hashString;
	ResourceManager* resourceManager;
	unsigned int curAnimationIndex;
	unsigned int nextAnimationIndex;

public:
	Blender();
	~Blender();

	void Init(bool timeBased);
	void Update(float time, unsigned int frameIndex);

	//getters
	std::vector<DirectX::XMFLOAT4X4> GetInverseBindPoses() { return boneOffsets; }
	std::vector<DirectX::XMFLOAT4X4> GetBonesWorlds() { return bonesWorlds; }

	//setters
	void SetAnimationSet(std::string key) { animationSet = resourceManager->GetAnimationSet(key); }
	void SetCurAnimationIndex(unsigned int curIndex) { curAnimationIndex = curIndex; }
	void SetNextAnimationIndex(unsigned int nextIndex) { nextAnimationIndex = nextIndex; }
};