// FBXLoader.cpp : Defines the exported functions for the DLL application.  
// Compile by using: cl /EHsc /DMATHLIBRARY_EXPORTS /LD MathLibrary.cpp  

#include "FBXLoader.h" 
#include <fbxsdk.h>

FbxManager* fbxManager = nullptr;

namespace FBXLoader
{
	bool Functions::FBXLoadFile(std::vector<Vertex> * outVerts, const char * filePath)
	{
		//if the FbxManager is not created. Crate it.
		if (!fbxManager)
		{
			
			fbxManager = FbxManager::Create();

			FbxIOSettings* settings = FbxIOSettings::Create(fbxManager, IOSROOT);
			fbxManager->SetIOSettings(settings);
		}


		FbxImporter* fbxImporter = FbxImporter::Create(fbxManager, "");
		FbxScene* fbxScene = FbxScene::Create(fbxManager, "");

		// the -1 is so that the plugin will detect the file format according to file suffix automatically.
		bool success = fbxImporter->Initialize(filePath, -1, fbxManager->GetIOSettings());
		if (!success) return false;

		success = fbxImporter->Import(fbxScene);
		if (!success) return false;

		//Destroy importer i
		fbxImporter->Destroy();

		//Create the root node as a handle for the rest of the FBX mesh
		FbxNode* rootNode = fbxScene->GetRootNode();

		//if the root node is not null
		if (rootNode)
		{
			//for every child node
			for (unsigned int i = 0; i < rootNode->GetChildCount(); ++i)
			{
				FbxNode* node = rootNode->GetChild(i);
				//skip child if null
				if (!node->GetNodeAttribute())
					continue;

				//get attribute type of the node
				FbxNodeAttribute::EType type = node->GetNodeAttribute()->GetAttributeType();

				//if it is not part of a mesh skip it 
				if (type != FbxNodeAttribute::eMesh)
					continue;

				FbxMesh* mesh = (FbxMesh*)node->GetNodeAttribute();

				FbxVector4* verts = mesh->GetControlPoints();

				for (unsigned int j = 0; j < mesh->GetPolygonCount(); ++j)
				{

					int numVerts = mesh->GetPolygonSize(j);

					//if the polgon is not a triangle wether the mesh is not triangulated or some other error
					if (numVerts != 3) return false;

					for (unsigned int k = 0; k < numVerts; ++k)
					{
						int iControlPointIndex = mesh->GetPolygonVertex(j, k);

						//if the requested vertex does not exists or the indices arguments have an invalid range
						if (iControlPointIndex < 0) return false;

						Vertex vert;
						vert.position.x = (float)verts[iControlPointIndex].mData[0];
						vert.position.y = (float)verts[iControlPointIndex].mData[1];
						vert.position.z = (float)verts[iControlPointIndex].mData[2];
						outVerts->push_back(vert);
					}
				}
			}
			return true;
		}
		return false;

	}

}



