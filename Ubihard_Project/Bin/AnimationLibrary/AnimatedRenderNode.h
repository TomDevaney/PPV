#pragma once
#include <DirectXMath.h>
#include <vector>

class AnimatedRenderNode
{
private:
	std::vector<DirectX::XMFLOAT4X4> inverseBindPoses;

public:

	//getters
	std::vector<DirectX::XMFLOAT4X4>& GetInverseBindPoses() { return inverseBindPoses; }

	//setters
	void SetInverseBindPoses(std::vector<DirectX::XMFLOAT4X4> poses) { inverseBindPoses = poses; }
};