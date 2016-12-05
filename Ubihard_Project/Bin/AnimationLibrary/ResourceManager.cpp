//#include "Animation.h"
//#include "AnimationSet.h"
//#include "BindPose.h"
#include "ForFBX.h"
#include "ResourceManager.h"
#include <fstream>
void ResourceManager::LoadInAnimationSet()
{
	//load in the  one skeleton
	LoadInSkeleton();

	//Load in multiple animations
	//for...
	LoadInAnimation();
}

void ResourceManager::LoadInSkeleton()
{
	std::ifstream bin;
	std::string skeletonPath;
	unsigned int sizeOfNames;
	unsigned int numOfBones;
	FriendlyIOSkeleton skeleton;

	skeletonPath = resourcesPath;
	skeletonPath += "Skeletons/Box.skel";

	bin.open(skeletonPath, std::ios::binary);

	if (bin.is_open())
	{
		//Read Header
		bin.read((char*)&numOfBones, sizeof(unsigned int));
		bin.read((char*)&sizeOfNames, sizeof(unsigned int));

		//resize based off of header
		skeleton.transforms.resize(numOfBones);
		skeleton.names.resize(sizeOfNames);
	
		//read in skeleton bones
		bin.read((char*)skeleton.transforms.data(), sizeof(FriendlyIOTransformNode) * numOfBones);

		//read in names
		bin.read((char*)skeleton.names.data(), sizeOfNames);
		
		bin.close();
	}

}

void ResourceManager::LoadInAnimation()
{
	std::ifstream bin;
	std::string animationPath;
	Animation animation;
	unsigned int numOfKeyFrames;
	unsigned int numOfBones;
	std::vector<KeyFrame> keyFrames;
	std::vector<Bone> bones;
	AnimType animType;
	float time;

	animationPath = resourcesPath;
	animationPath += "Animations/Box.anim";

	bin.open(animationPath, std::ios::binary);

	if (bin.is_open())
	{
		//read header
		bin.read((char*)&numOfKeyFrames, sizeof(unsigned int));
		bin.read((char*)&numOfBones, sizeof(unsigned int));

		//read keyframes
		keyFrames.resize(numOfKeyFrames);
		bones.resize(numOfBones);

		for (unsigned int i = 0; i < numOfKeyFrames; ++i)
		{
			float keyFrameTime;

			bin.read((char*)bones.data(), sizeof(Bone) * numOfBones);
			bin.read((char*)&keyFrameTime, sizeof(float));

			keyFrames[i].SetTime(keyFrameTime);
			keyFrames[i].SetBones(bones);
		}

		//bin.read((char*)keyFrames.data(), sizeof(KeyFrame) * numOfKeyFrames);

		//write out animtype
		bin.read((char*)&animType, sizeof(AnimType));

		//read in time
		bin.read((char*)&time, sizeof(float));

		animation.Init(animType, time, keyFrames);

		//clear stuff
		//keyFrames.clear();
		//bones.clear();

		bin.close();
	}
}