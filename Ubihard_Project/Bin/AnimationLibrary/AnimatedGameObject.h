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
	void Init(std::string animSet);
	void Update();

	//setters
	void SetRenderNode(AnimatedRenderNode* node) { renderNode = node; }
	void SetCurFrame(unsigned int num) { curFrame = num; }
};