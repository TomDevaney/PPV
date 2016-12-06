#include "Blender.h"

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

void Blender::Init()
{
	//make interpolator
	curAnim = new Interpolator();
	curAnim->SetAnimation(animationSet->GetAnimation(0));
}

void Blender::Update(float time, unsigned int frameIndex)
{
	std::vector<Bone> bones;

	curAnim->SetAnimation(animationSet->GetAnimation(0));
	curAnim->SetCurFrame(frameIndex % animationSet->GetAnimation(0)->GetNumKeyFrames());
	curAnim->Update(time);

	bones = curAnim->GetBones();

	boneOffsets.clear();
	bonesWorlds.clear();

	for (int i = 0; i < bones.size(); ++i)
	{
		DirectX::XMFLOAT4X4 boneOffset;

		DirectX::XMMATRIX inverseBindPose = DirectX::XMLoadFloat4x4(&bones[i].GetInverseBindPose());
		DirectX::XMMATRIX boneWorld = DirectX::XMLoadFloat4x4(&bones[i].GetWorld());
		//DirectX::XMMATRIX transformWorld = DirectX::XMLoadFloat4x4(&animationSet->GetSkeleton().GetBones()[i].world);

		XMStoreFloat4x4(&boneOffset, DirectX::XMMatrixMultiply(inverseBindPose, boneWorld));

		boneOffsets.push_back(boneOffset); //this is actually a container for boneoffsets for 
		bonesWorlds.push_back(bones[i].GetWorld());
	}
}