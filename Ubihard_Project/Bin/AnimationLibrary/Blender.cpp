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

	for (unsigned int i = 0; i <animationSet->GetSkeleton().GetNumBones(); ++i)
	{
		DirectX::XMFLOAT4X4 boneOffset;
		DirectX::XMMATRIX boneWorld;
		DirectX::XMFLOAT4X4 boneWorldFloat;
		DirectX::XMMATRIX transformInverseBindPose;
		DirectX::XMMATRIX inverseBindPose;


		if (i < bones.size())
		{
			boneWorld = DirectX::XMLoadFloat4x4(&bones[i].GetWorld());
			inverseBindPose = DirectX::XMLoadFloat4x4(&bones[i].GetInverseBindPose());
		}
		else
		{
			boneWorld = DirectX::XMMatrixIdentity();
			inverseBindPose = DirectX::XMMatrixIdentity();
		}

		//transformInverseBindPose = DirectX::XMLoadFloat4x4(&animationSet->GetSkeleton().GetBones()[i].world);

		DirectX::XMStoreFloat4x4(&boneOffset, DirectX::XMMatrixMultiply(inverseBindPose, boneWorld));
		DirectX::XMStoreFloat4x4(&boneWorldFloat, DirectX::XMMatrixTranspose(boneWorld));

		boneOffsets.push_back(boneOffset); //this is actually a container for boneoffsets for 
		bonesWorlds.push_back(boneWorldFloat);
	}
}