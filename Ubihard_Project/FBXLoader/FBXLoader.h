// FBXLoader.h - Contains declaration of the FBXLoadFile Function
#pragma once  
//https://msdn.microsoft.com/en-us/library/ms235636.aspx
#ifdef FBXLOADER_EXPORTS  
#define FBXLOADER_API __declspec(dllexport)   
#else  
#define FBXLOADER_API __declspec(dllimport)   
#endif  

#include <vector>
#include "..\Ubihard_Project\ShaderStructures.h"
namespace FBXLoader
{
	// This class is exported from the FBXLOADER.dll  
	class Functions
	{
	public:
		// Returns a + b  
		static FBXLOADER_API bool FBXLoadFile(std::vector<Vertex> * outVerts, std::vector<unsigned int> * outIndices,  const char * filePath);
	};
}