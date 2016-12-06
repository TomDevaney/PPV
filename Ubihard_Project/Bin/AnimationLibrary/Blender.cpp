#include "Blender.h"

Blender::Blender()
{
	hashString = HashString::GetSingleton();
	resourceManager = ResourceManager::GetSingleton();
}