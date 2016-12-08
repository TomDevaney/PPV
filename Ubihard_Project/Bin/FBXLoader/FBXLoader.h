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
		static FBXLOADER_API bool FBXLoadExportFileBind(const char * inFilePath, const char * name, const char* animationName);
		static FBXLOADER_API bool FBXLoadExportFileBasic(const char * inFilePath, const char * name);
		static FBXLOADER_API bool FBXLoadExportAnimation(const char * inFilePath, const char * name, const char* animationName);
		static FBXLOADER_API bool FBXLoadExportSkeleton(const char * inFilePath, const char * name);
		static FBXLOADER_API bool FBXLoadExportMesh(const char * inFilePath, const char * name);
	};
}