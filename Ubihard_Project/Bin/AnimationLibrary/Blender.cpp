#include "Blender.h"
#include <map>

Blender::Blender()
{
	hashString = HashString::GetSingleton();
	resourceManager = ResourceManager::GetSingleton();
}

Blender::~Blender()
{
	delete curAnim;
	//delete nextAnim;
}

void Blender::Init(bool timeBased)
{
	//make interpolator
	curAnim = new Interpolator();
	curAnim->SetAnimation(animationSet->GetAnimation(curAnimationIndex));
	//curAnim->SetCurFrame(0);
	curAnim->SetIsTimeBased(timeBased);
}

void Blender::Update(float time, unsigned int frameIndex) // i just use frameIndex for bear, so if its 0 and time isn't 0, don't update
{
	std::vector<Bone>* bones;

	curAnim->SetAnimation(animationSet->GetAnimation(curAnimationIndex));

	if (!time)
	{
		curAnim->SetCurFrame(frameIndex % animationSet->GetAnimation(0)->GetNumKeyFrames());
	}

	curAnim->Update(time);

	bones = curAnim->GetBones();

	boneOffsets.clear();
	bonesWorlds.clear();

	for (unsigned int i = 0; i < animationSet->GetSkeleton().GetNumBones(); ++i)
	{
		DirectX::XMFLOAT4X4 boneOffset;
		DirectX::XMMATRIX boneWorld;
		DirectX::XMFLOAT4X4 boneWorldFloat;
		DirectX::XMMATRIX inverseBindPose;
		 
		boneWorld = DirectX::XMLoadFloat4x4(&(*bones)[i].GetWorld());

		inverseBindPose = DirectX::XMLoadFloat4x4(&animationSet->GetSkeleton().GetInverseBindPose(i));

		DirectX::XMStoreFloat4x4(&boneOffset, DirectX::XMMatrixTranspose(DirectX::XMMatrixMultiply(inverseBindPose, boneWorld)));
		DirectX::XMStoreFloat4x4(&boneWorldFloat, boneWorld);

		boneOffsets.push_back(boneOffset); 
		bonesWorlds.push_back(boneWorldFloat);
	}
}