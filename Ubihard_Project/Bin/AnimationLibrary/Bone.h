#pragma once
#include <DirectXMath.h>
#include <string>

class Bone
{
private:
	std::string name;
	DirectX::XMFLOAT4X4 world; //original matrix... I think
	DirectX::XMFLOAT4X4 boneOffset; //How far it strayed from original matrix. Used to get new animation state
	DirectX::XMFLOAT4X4 local;
public:
	//void Init(DirectX::XMFLOAT4X4 worldM, DirectX::XMFLOAT4X4 offset, std::string nam);

	//getters
	DirectX::XMFLOAT4X4 GetWorld() { return world; }
	DirectX::XMFLOAT4X4 GetLocal() { return local; }
	DirectX::XMFLOAT4X4 GetBoneOffset() { return boneOffset; }

	//setters
	void SetWorld(DirectX::XMFLOAT4X4 matrix) { world = matrix; }
	void BoneOffset(DirectX::XMFLOAT4X4 matrix) { boneOffset = matrix; }
	void SetLocal(DirectX::XMFLOAT4X4 matrix) { local = matrix; }
	void SetName(std::string nam) { name = nam; }
};