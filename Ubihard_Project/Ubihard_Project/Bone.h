#pragma once
#include <DirectXMath.h>

class Bone
{
private:
	DirectX::XMFLOAT4X4 world; //original matrix... I think
	DirectX::XMFLOAT4X4 boneOffset; //How far it strayed from original matrix. Used to get new animation state
	DirectX::XMFLOAT4X4 local;
	//joint names
public:
	void Init(DirectX::XMFLOAT4X4 worldM, DirectX::XMFLOAT4X4 offset);

	//getters
	DirectX::XMFLOAT4X4 GetWorld() { return world; }
	DirectX::XMFLOAT4X4 GetLocal() { return local; }
	DirectX::XMFLOAT4X4 GetBoneOffset() { return boneOffset; }
};