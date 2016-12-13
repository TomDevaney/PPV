//#include "Animation.h"
//#include "AnimationSet.h"
//#include "BindPose.h"
#include <fstream>
#include <Windows.h>
#include "ForFBX.h"
#include "ResourceManager.h"

ResourceManager* ResourceManager::singleton = 0;

ResourceManager::ResourceManager()
{
	hashString = HashString::GetSingleton();
	animationSetIndex = 0;
}

ResourceManager::~ResourceManager()
{
	hashString->CleanUp();
}

void ResourceManager::CleanUp()
{
	delete singleton;
	singleton = nullptr;
}

void ResourceManager::LoadInAnimationSet()
{
	WIN32_FIND_DATA fileData, folderData;
	Skeleton skeleton;
	Animation animation;
	HANDLE hFolderFind, hFileFind;
	std::wstring resourcesFolderPath;

	resourcesFolderPath = resourcesPath;
	resourcesFolderPath += L"*";

	hFolderFind = ::FindFirstFile(resourcesFolderPath.c_str(), &folderData);

	if (hFolderFind != INVALID_HANDLE_VALUE)
	{
		do // for every folder in resources folder
		{
			//every folder is one animation set
			AnimationSet animationSet;

			if (folderData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				std::wstring filePath = resourcesPath;
				filePath += folderData.cFileName;
				filePath += L"/*.skel";

				//load in the  one skeleton
				hFileFind = ::FindFirstFile(filePath.c_str(), &fileData);

				if (hFileFind != INVALID_HANDLE_VALUE)
				{
						//set file path to the skel file
						filePath = resourcesPath;
						filePath += folderData.cFileName;
						filePath += L"/";
						filePath += fileData.cFileName;

						skeleton = LoadInSkeleton(filePath);
						animationSet.SetSkeleton(skeleton);
				}

				filePath = resourcesPath;
				filePath += folderData.cFileName;
				filePath += L"/*.anim";

				hFileFind = ::FindFirstFile(filePath.c_str(), &fileData);

				if (hFileFind != INVALID_HANDLE_VALUE)
				{
					do //for every file in folder
					{
						//set file path to the anim file
						filePath = resourcesPath;
						filePath += folderData.cFileName;
						filePath += L"/";
						filePath += fileData.cFileName;

						//Load in multiple animations
						animation = LoadInAnimation(filePath);

						//initialize animation set
						animationSet.AddAnimation(animation);
					} while (::FindNextFile(hFileFind, &fileData));

					FindClose(hFileFind);
				}
			}

			if (hFileFind != INVALID_HANDLE_VALUE) //only add animation set if a file was loaded in
			{
				//add set to animation set and increment inedx
				animationSets[animationSetIndex++] = animationSet;

				//add to hash string
				size_t length = WideCharToMultiByte(CP_ACP, 0, &folderData.cFileName[0], -1, NULL, 0, NULL, NULL);
				std::string hashValue(length, 0);
				WideCharToMultiByte(CP_UTF8, 0, &folderData.cFileName[0], 260, &hashValue[0], (int)length, NULL, NULL);

				hashString->Insert(hashValue);
			}

		} while (::FindNextFile(hFolderFind, &folderData));

		FindClose(hFolderFind);
	}
}



Skeleton ResourceManager::LoadInSkeleton(std::wstring path)
{
	std::ifstream bin;
	FriendlyIOSkeleton skeleton;
	unsigned int sizeOfNames;
	unsigned int numOfBones;
	unsigned int nameSize;

	bin.open(path, std::ios::binary);

	if (bin.is_open())
	{
		//Read Header
		bin.read((char*)&numOfBones, sizeof(unsigned int));
		bin.read((char*)&sizeOfNames, sizeof(unsigned int));

		//resize based off of header
		skeleton.transforms.resize(numOfBones);
		skeleton.inverseBindPoses.resize(numOfBones);
		skeleton.names.resize(sizeOfNames);
	
		//read in skeleton bones
		bin.read((char*)skeleton.transforms.data(), sizeof(FriendlyIOTransformNode) * numOfBones);

		//read in skeleton names
		bin.read((char*)skeleton.names.data(), sizeOfNames);

		//read in inverse bind poses
		bin.read((char*)skeleton.inverseBindPoses.data(), sizeof(DirectX::XMFLOAT4X4) * numOfBones);

		bin.close();
	}

	Skeleton nonFriendlySkeleton;

	nonFriendlySkeleton.Init(skeleton.transforms, skeleton.names, skeleton.inverseBindPoses);

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
			float tweenTime;

			bin.read((char*)bones.data(), sizeof(Bone) * numOfBones);
			bin.read((char*)&keyFrameTime, sizeof(float));
			bin.read((char*)&tweenTime, sizeof(float));

			keyFrames[i].SetTime(keyFrameTime);
			keyFrames[i].SetTweenTime(tweenTime);
			keyFrames[i].SetBones(bones);
		}

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