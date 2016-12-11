#include "Interpolator.h"

// i use the curFrame with the next frame to get a interpolated frame
// i store the results in the vector of bones
// this way the animated game object will be able to get this info from me and send it to the aimatedrendernode
// which will send this data to the game engine
using namespace DirectX;

Interpolator::Interpolator()
{
	//betweenKeyFrame = nullptr;
	bones = new std::vector<Bone>();
	curTime = 0;
	frameTime = 0;
	timeBased = false;
	prevFrame = 0;
	nextFrame = 1;
	curFrame = 0;
}

Interpolator::~Interpolator()
{
	//delete animation;
	delete bones;
}

AnimType Interpolator::Update(float time)
{
	if (timeBased)
	{
		float tweenTime = 0;

		betweenKeyFrame.ClearBones();

		//update time
		frameTime += time;
		curTime += time;
		tweenTime = animation->GetFrame(nextFrame)->GetTime() - animation->GetFrame(prevFrame)->GetTime();

		if (tweenTime < 0.0f)
		{
			tweenTime = animation->GetFrame(1)->GetTime();
		}

		//this pretty much moves both nextframe and prev frame up one
		while (frameTime > tweenTime) // if previous frame passed next frame
		{
			prevFrame = nextFrame; //set previous frame equal to next frame
			frameTime -= tweenTime; //subtract tween time from frame time
			++nextFrame; //increment next frame
		}

		//error check frame indices
		if (nextFrame == animation->GetNumKeyFrames())
		{
			nextFrame = 0;
		}
		else if (prevFrame == animation->GetNumKeyFrames())
		{
			prevFrame = 0;
		}

		//update between key frame
		float delta = frameTime / tweenTime;

		Interpolate(animation->GetFrame(prevFrame), animation->GetFrame(nextFrame), delta);

		//get bones from current frame
		bones->clear();
		for (unsigned int i = 0; i < betweenKeyFrame.GetBones().size(); ++i)
		{
			bones->push_back(betweenKeyFrame.GetBone(i));
		}

	}
	else
	{
		//get bones from current frame
		KeyFrame* curKeyFrame;

		curKeyFrame = animation->GetFrame(curFrame);

		bones->clear();
		for (unsigned int i = 0; i < curKeyFrame->GetBones().size(); ++i)
		{
			bones->push_back(curKeyFrame->GetBone(i));
		}
	}

	return AnimType::RUN_ONCE;
}

//private helper functions
void Interpolator::Interpolate(KeyFrame* previous, KeyFrame* next, float ratio)
{
	for (int i = 0; i < previous->GetBones().size(); ++i)
	{
		XMFLOAT4X4 newWorld;
		Bone newBone;

		XMVECTOR quarternion = XMQuaternionSlerp(XMQuaternionRotationMatrix(XMLoadFloat4x4(&previous->GetBone(i).GetWorld())), XMQuaternionRotationMatrix(XMLoadFloat4x4(&next->GetBone(i).GetWorld())), ratio);
		
		XMFLOAT4X4 prevBone = previous->GetBone(i).GetWorld();
		XMVECTOR prevTranslation = XMVectorSet(prevBone._41, prevBone._42, prevBone._43, prevBone._44);

		XMFLOAT4X4 nextBone = next->GetBone(i).GetWorld();
		XMVECTOR nextTranslation = XMVectorSet(nextBone._41, nextBone._42, nextBone._43, nextBone._44);

		XMVECTOR newTranslation = XMVectorLerp(prevTranslation, nextTranslation, ratio);

		XMMATRIX resultMatrix = XMMatrixAffineTransformation({ 1, 1, 1, 1 }, { 0, 0, 0, 0 }, quarternion, newTranslation);

		newBone = previous->GetBone(i);
		XMStoreFloat4x4(&newWorld, resultMatrix);
		newBone.SetWorld(newWorld);

		betweenKeyFrame.InsertBone(newBone);
	}

}