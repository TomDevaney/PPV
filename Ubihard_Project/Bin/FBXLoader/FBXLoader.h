// FBXLoader.h - Contains declaration of the FBXLoadFile Function
#pragma once  
//https://msdn.microsoft.com/en-us/library/ms235636.aspx
#ifdef FBXLOADER_EXPORTS  
#define FBXLOADER_API __declspec(dllexport)   
#else  
#define FBXLOADER_API __declspec(dllimport)   
#endif  

#include <vector>
#include "../ShaderStructures/ShaderStructures.h"
#include "../AnimationLibrary/ForFBX.h"

namespace FBXLoader
{
	// This class is exported from the FBXLOADER.dll  
	class Functions
	{
	public:
		// Returns a + b  
		//static FBXLOADER_API bool FBXLoadFile(std::vector<Vertex> * outVerts, std::vector<unsigned int> * outIndices, std::vector<XMFLOAT4X4> *outBonePos, const char * filePath);
		static FBXLOADER_API bool FBXLoadExportFileBind(const char * inFilePath, const char * name, const char* animationName, bool justAnimation);
		static FBXLOADER_API bool FBXLoadExportFileBasic(const char * inFilePath, const char * name);
	};
}