#pragma once
#include <vector>
#include <DirectXMath.h>
#include "AnimationSet.h"
#include "AnimationIncludes.h"
#include "Interpolator.h"
#include "HashString.h"
#include "ResourceManager.h"
#include <DirectXMath.h>

class Blender
{
private:
	AnimationSet* animationSet;
	BlendInfo blendInfo;
	std::vector<DirectX::XMFLOAT4X4> boneOffsets;
	std::vector<DirectX::XMFLOAT4X4> bonesWorlds;
	Interpolator* curInterpolator;
	Interpolator* nextInterpolator;
	HashString* hashString;
	ResourceManager* resourceManager;
	//unsigned int curAnimationIndex;
	//int nextAnimationIndex;
	KeyFrame blendedKeyFrame;
	float curBlendTime;

	//private helper functions
	void Interpolate(KeyFrame* previous, KeyFrame* next, float ratio);
	//void MakeCurAnimation(bool timeBased);
	//void MakeNextAnimation(bool timeBased);

public:
	Blender();
	~Blender();

	void Init(bool timeBased, unsigned int curAnimIndex, int nextAnimIndex);
	void Update(float time, unsigned int frameIndex);

	//getters
	std::vector<DirectX::XMFLOAT4X4> GetInverseBindPoses() { return boneOffsets; }
	std::vector<DirectX::XMFLOAT4X4> GetBonesWorlds() { return bonesWorlds; }

	//setters
	void SetCurAnimationIndex(unsigned int curIndex);
	void SetNextAnimationIndex(unsigned int nextIndex);
	void SetAnimationSet(std::string key) { animationSet = resourceManager->GetAnimationSet(key); }
	void SetBlendInfo(BlendInfo info) { blendInfo = info; }
};