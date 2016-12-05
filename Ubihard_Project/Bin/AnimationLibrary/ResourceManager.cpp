//#include "Animation.h"
//#include "AnimationSet.h"
//#include "BindPose.h"
#include "ForFBX.h"
#include "ResourceManager.h"
#include <fstream>

void ResourceManager::LoadInSkeleton()
{
	std::ifstream bin;
	std::string skeletonPath;
	unsigned int sizeOfNames;
	unsigned int numOfBones;
	FriendlyIOSkeleton skeleton;

	skeletonPath = skeletonsPath;
	skeletonPath += "Box.skel";

	bin.open(skeletonPath, std::ios::binary);

	if (bin.is_open())
	{
		//Read Header
		bin.read((char*)&numOfBones, sizeof(unsigned int));
		bin.read((char*)&sizeOfNames, sizeof(unsigned int));

		//resize based off of header
		skeleton.transforms.resize(numOfBones);
	
		//read in skeleton

		//bones
		bin.read((char*)&skeleton.transforms, sizeof(FriendlyIOTransformNode) * numOfBones);

		//names
		skeleton.names.resize(sizeOfNames);
		bin.read((char*)skeleton.names.data(), sizeOfNames);
	}

	bin.close();
}