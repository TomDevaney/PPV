#include "Interpolator.h"

// i use the curFrame with the next frame to get a interpolated frame
// i store the results in the vector of bones
// this way the animated game object will be able to get this info from me and send it to the aimatedrendernode
// which will send this data to the game engine

AnimType Interpolator::Update(float time)
{
	 KeyFrame* currentFrame = animation->GetFrame(curFrame);

	 //get bones from current frame
	for (unsigned int i = 0; i < currentFrame->GetBones().size(); ++i)
	{
		bones.push_back(*currentFrame->GetBone(i));
	}

	return AnimType::RUN_ONCE;
}