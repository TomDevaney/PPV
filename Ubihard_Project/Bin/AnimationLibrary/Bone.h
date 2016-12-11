#pragma once
#include <DirectXMath.h>
#include <string>

class Bone
{
private:
	char name[100] = { '\0' };
	//DirectX::XMFLOAT4X4 inverseBindPose; //original matrix... I think
	DirectX::XMFLOAT4X4 boneOffset; //probably won't need because I can't calculate on animation side
	//DirectX::XMFLOAT4X4 inverseBindPose;
	DirectX::XMFLOAT4X4 local;
	DirectX::XMFLOAT4X4 world;
public:
	//void Init(DirectX::XMFLOAT4X4 worldM, DirectX::XMFLOAT4X4 offset, std::string nam);

	//getters
	//DirectX::XMFLOAT4X4 GetInverseBindPose() { return inverseBindPose; }
	DirectX::XMFLOAT4X4 GetLocal() { return local; }
	DirectX::XMFLOAT4X4 GetWorld() { return world; }
	DirectX::XMFLOAT4X4 GetBoneOffset() { return boneOffset; }
	std::string GetName() { return name; }

	//setters
	//void SetInverseBindPose(DirectX::XMFLOAT4X4 matrix) { inverseBindPose = matrix; }
	void BoneOffset(DirectX::XMFLOAT4X4 matrix) { boneOffset = matrix; }
	void SetLocal(DirectX::XMFLOAT4X4 matrix) { local = matrix; }
	void SetWorld(DirectX::XMFLOAT4X4 matrix) { world = matrix; }
	void SetName(std::string nam) { for (int i = 0; i < nam.size(); ++i) { name[i] = nam[i]; } }
};