//#include "Animation.h"
//#include "AnimationSet.h"
//#include "BindPose.h"
#include <fstream>
#include <Windows.h>
#include "ForFBX.h"
#include "ResourceManager.h"

ResourceManager::ResourceManager()
{
	hashString = HashString::GetSingleton();
	animationSetIndex = 0;
}

ResourceManager::~ResourceManager()
{
	hashString->CleanUp();
}

void ResourceManager::LoadInAnimationSet()
{
	WIN32_FIND_DATA fileData, folderData;
	Skeleton skeleton;
	Animation animation;
	HANDLE hFolderFind, hFileFind;

	//skeletonPath += L"/*.*";
	hFolderFind = ::FindFirstFile(resourcesPath.c_str(), &folderData);
	
	if (hFolderFind != INVALID_HANDLE_VALUE)
	{
		do // for every folder in resources folder
		{
			//every folder is one animation set
			AnimationSet animationSet;


			if (folderData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				std::wstring filePath = folderData.cFileName;
				filePath += L"/*.skel";

				//load in the  one skeleton
				//skeleton = LoadInSkeleton(filePath);
				//animationSet.SetSkeleton(&skeleton);

				filePath = folderData.cFileName;
				filePath += L"/*.anim";

				hFileFind = FindFirstFile(folderData.cFileName, &fileData);

				if (hFileFind != INVALID_HANDLE_VALUE)
				{
					do //for every file in folder
					{
						//Load in multiple animations
						//animation = LoadInAnimation(fileData.cFileName);

						//initialize animation set
						//animationSet.AddAnimation(animation);

					} while (FindNextFile(hFolderFind, &fileData)); 

					FindClose(hFileFind);
				}
			}

			animationSets[animationSetIndex] = animationSet;

			//add to hash string
			//hashString->Insert();

		} while (FindNextFile(hFolderFind, &folderData));

		FindClose(hFolderFind);
	}




}

Skeleton ResourceManager::LoadInSkeleton(std::wstring path)
{
	std::ifstream bin;
	FriendlyIOSkeleton skeleton;
	unsigned int sizeOfNames;
	unsigned int numOfBones;

	bin.open(path, std::ios::binary);

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

	Skeleton nonFriendlySkeleton;

	nonFriendlySkeleton.Init(skeleton.transforms, skeleton.names);

	return nonFriendlySkeleton;
}

Animation ResourceManager::LoadInAnimation(std::wstring path)
{
	std::ifstream bin;
	Animation animation;
	std::vector<KeyFrame> keyFrames;
	std::vector<Bone> bones;
	AnimType animType;
	float time;
	unsigned int numOfKeyFrames;
	unsigned int numOfBones;

	bin.open(path, std::ios::binary);

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

		//close fstream
		bin.close();
	}

	return animation;
}