#pragma once
#include <DirectXMath.h>
#include <vector>

class AnimatedRenderNode
{
private:
	std::vector<DirectX::XMFLOAT4X4> boneOffsets;
	std::vector<DirectX::XMFLOAT4X4> bonesWorlds;

public:

	//getters
	std::vector<DirectX::XMFLOAT4X4>& GetBoneOffsets() { return boneOffsets; }
	std::vector<DirectX::XMFLOAT4X4>& GetBonesWorlds() { return bonesWorlds; }

	//setters
	void SetInverseBindPoses(std::vector<DirectX::XMFLOAT4X4> poses) { boneOffsets = poses; }
	void SetBonesWorlds(std::vector<DirectX::XMFLOAT4X4> worlds) { bonesWorlds = worlds; }
};