#include "Interpolator.h"

// i use the curFrame with the next frame to get a interpolated frame
// i store the results in the vector of bones
// this way the animated game object will be able to get this info from me and send it to the aimatedrendernode
// which will send this data to the game engine

Interpolator::Interpolator()
{
	bones = new std::vector<Bone>();
	totalBlendTime = 0;
	timeBased = false;
}

Interpolator::~Interpolator()
{
	//delete animation;
	delete bones;
}

AnimType Interpolator::Update(float time)
{
	KeyFrame* currentFrame = nullptr;
	
	//set pointer to current frame
	currentFrame = animation->GetFrame(curFrame);

	//update time
	totalBlendTime += time;

	//do automatic animation based off of time
	if (timeBased)
	{
		if (currentFrame->GetTime() < totalBlendTime)
		{
			++curFrame;

			if (curFrame >= animation->GetNumKeyFrames())
			{
				curFrame = 0;
				totalBlendTime = 0;
			}

			currentFrame = animation->GetFrame(curFrame);
		}
		//if (currentFrame->GetTime() > totalBlendTime)
		//{
		//	++curFrame %= animation->GetNumKeyFrames();
		//	currentFrame = animation->GetFrame(curFrame);
		//}
	}

	bones->clear();


	//get bones from current frame
	for (unsigned int i = 0; i < currentFrame->GetBones().size(); ++i)
	{
		bones->push_back(currentFrame->GetBone(i));
	}

	return AnimType::RUN_ONCE;
}