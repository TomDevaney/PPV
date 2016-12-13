#pragma once
#include <vector>
#include <DirectXMath.h>
#include "Blender.h"
#include "AnimatedRenderNode.h"

class AnimatedGameObject
{
private:
	Blender blender;
	AnimatedRenderNode* renderNode;
	unsigned int curFrame;

public:
	void Init(std::string animSet, unsigned int curAnimationIndex, int nextAnimationIndex, bool timeBased);
	void Update(float deltaTime);

	//setters
	void SetRenderNode(AnimatedRenderNode* node) { renderNode = node; }
	void SetCurFrame(unsigned int num) { curFrame = num; }
	void SetBlendInfo(BlendInfo info) { blender.SetBlendInfo(info); }
	void SetCurAnimation(unsigned int curAnimationIndex) { blender.SetCurAnimationIndex(curAnimationIndex); }
	void SetNextAnimation(unsigned int nextAnimationIndex) { blender.SetNextAnimationIndex(nextAnimationIndex); }
	//void CreateCurAnimation(bool timeBased) { blender.MakeCurAnimation(timeBased); }
	//void CreateNextAnimation(bool timeBased) { blender.MakeNextAnimation(timeBased); }
};