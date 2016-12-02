#include "Bone.h"

void Bone::Init(DirectX::XMFLOAT4X4 worldM, DirectX::XMFLOAT4X4 offset)
{
	world = worldM;
	boneOffset = offset;
}