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

void Blender::Update(float time)
{
	std::vector<Bone>* bones;

	curAnim->SetAnimation(animationSet->GetAnimation(0));
	curAnim->SetCurFrame(0);
	curAnim->Update(time);

	bones = curAnim->GetBones();

	inverseBindPoses.clear();

	for (int i = 0; i < bones->size(); ++i)
	{
		inverseBindPoses.push_back((*bones)[i].GetInverseBindPose());
	}
}