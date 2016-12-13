#include "AnimatedGameObject.h"

void AnimatedGameObject::Init(std::string animSet, unsigned int curAnimationIndex, int nextAnimationIndex, bool timeBased)
{
	//set blender's anim state
	blender.SetAnimationSet(animSet);
	//blender.SetCurAnimationIndex(curAnimationIndex);
	//blender.SetNextAnimationIndex(nextAnimationIndex);
	blender.Init(timeBased, curAnimationIndex, nextAnimationIndex); //this sets the blenders interpolator
}

void AnimatedGameObject::Update(float deltaTime)
{
	//update blender
	blender.Update(deltaTime, curFrame); //just for now I will set its time to 0.0f

	//send animatedrendernode inverse bind matrices
	renderNode->SetInverseBindPoses(blender.GetInverseBindPoses());
	renderNode->SetBonesWorlds(blender.GetBonesWorlds());
}