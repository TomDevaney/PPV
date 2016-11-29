// FBXLoader.cpp : Defines the exported functions for the DLL application.  
// Compile by using: cl /EHsc /DMATHLIBRARY_EXPORTS /LD MathLibrary.cpp  

#include "FBXLoader.h" 
#include <fbxsdk.h>

FbxManager* fbxManager = nullptr;

namespace FBXLoader
{
	void LoadUV(FbxMesh* iMesh, int iCtrlPoint, int iTextureUVIndex, int iUVLayer, Vertex& iVert)
	{
		//invalid paramaters
		if (iUVLayer >= 2 || iMesh->GetElementUVCount() <= iUVLayer) return;

		//get the uv layer from mesh
		FbxGeometryElementUV* vertUV = iMesh->GetElementUV(iUVLayer);

		//switch on mapping mode
		switch (vertUV->GetMappingMode())
		{
		case FbxGeometryElement::eByControlPoint:
			switch (vertUV->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				iVert.uv.x = (float)(vertUV->GetDirectArray().GetAt(iCtrlPoint).mData[0]);
				iVert.uv.y = (float)(vertUV->GetDirectArray().GetAt(iCtrlPoint).mData[1]);
			}
			break;

			case FbxGeometryElement::eIndexToDirect:
			{
				int index = vertUV->GetIndexArray().GetAt(iCtrlPoint);
				iVert.uv.x = (float)(vertUV->GetDirectArray().GetAt(index).mData[0]);
				iVert.uv.y = (float)(vertUV->GetDirectArray().GetAt(index).mData[1]);
			}
			break;

			default:
				break;
			}

		case FbxGeometryElement::eByPolygonVertex:
			switch (vertUV->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			case FbxGeometryElement::eIndexToDirect:
			{
				iVert.uv.x = (float)(vertUV->GetDirectArray().GetAt(iTextureUVIndex).mData[0]);
				iVert.uv.y = (float)(vertUV->GetDirectArray().GetAt(iTextureUVIndex).mData[1]);
			}
			break;

			default:
				break;
			}
		}
		iVert.uv.y = 1.0f - iVert.uv.y;
	}

	void LoadNormal(FbxMesh* mesh, int iCtrlPoint, int iVertCounter, Vertex& vert)
	{
		FbxGeometryElementNormal * vertNormal = mesh->GetElementNormal();
		switch (vertNormal->GetMappingMode())
		{
		case FbxGeometryElement::eByControlPoint:
			switch (vertNormal->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				vert.normal.x = (float)(vertNormal->GetDirectArray().GetAt(iCtrlPoint).mData[0]);
				vert.normal.y = (float)(vertNormal->GetDirectArray().GetAt(iCtrlPoint).mData[1]);
				vert.normal.z = (float)(vertNormal->GetDirectArray().GetAt(iCtrlPoint).mData[2]);
			}
			break;

			case FbxGeometryElement::eIndexToDirect:
			{
				int index = vertNormal->GetIndexArray().GetAt(iCtrlPoint);
				vert.normal.x = (float)(vertNormal->GetDirectArray().GetAt(index).mData[0]);
				vert.normal.y = (float)(vertNormal->GetDirectArray().GetAt(index).mData[1]);
				vert.normal.z = (float)(vertNormal->GetDirectArray().GetAt(index).mData[2]);
			}
			break;

			default:
				break;
			}

		case FbxGeometryElement::eByPolygonVertex:
			switch (vertNormal->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				vert.normal.x = (float)(vertNormal->GetDirectArray().GetAt(iVertCounter).mData[0]);
				vert.normal.y = (float)(vertNormal->GetDirectArray().GetAt(iVertCounter).mData[1]);
				vert.normal.z = (float)(vertNormal->GetDirectArray().GetAt(iVertCounter).mData[2]);
			}
			break;

			case FbxGeometryElement::eIndexToDirect:
			{
				int index = vertNormal->GetIndexArray().GetAt(iVertCounter);
				vert.normal.x = (float)(vertNormal->GetDirectArray().GetAt(index).mData[0]);
				vert.normal.y = (float)(vertNormal->GetDirectArray().GetAt(index).mData[1]);
				vert.normal.z = (float)(vertNormal->GetDirectArray().GetAt(index).mData[2]);
			}
			break;

			default:
				break;
			}
		}
		vert.normal.x = -vert.normal.x;
	}

	int FindVertex(const Vertex& inTargetVertex, const std::vector<Vertex>& uniqueVertices)
	{
		for (unsigned int i = 0; i < uniqueVertices.size(); ++i)
			if (inTargetVertex == uniqueVertices[i])
				return i;

		return -1;
	}

	void ElimanateDuplicates(std::vector<Vertex>& mVertices, std::vector<unsigned int>& mIndices)
	{
		// First get a list of unique vertices
		std::vector<Vertex> uniqueVertices;
		for (unsigned int i = 0; i < mVertices.size(); ++i)
			//iff unique add it to vector
			if (FindVertex(mVertices[i], uniqueVertices) == -1)
				uniqueVertices.push_back(mVertices[i]);

		// Now we update the index buffer
		for (unsigned int i = 0; i < mVertices.size(); ++i)
			mIndices[i] = FindVertex(mVertices[i], uniqueVertices);
	
		mVertices.clear();
		mVertices = uniqueVertices;
		uniqueVertices.clear();
	}

	bool Functions::FBXLoadFile(std::vector<Vertex> * outVerts, std::vector<unsigned int> * outIndices, const char * filePath)
	{
		//if the FbxManager is not created. Create it.
		if (!fbxManager)
		{
			fbxManager = FbxManager::Create();

			FbxIOSettings* settings = FbxIOSettings::Create(fbxManager, IOSROOT);
			fbxManager->SetIOSettings(settings);
		}


		FbxImporter* fbxImporter = FbxImporter::Create(fbxManager, "");
		FbxScene* fbxScene = FbxScene::Create(fbxManager, "");

		// the -1 is so that the plugin will detect the file format according to file suffix automatically.
		if (!fbxImporter->Initialize(filePath, -1, fbxManager->GetIOSettings())) return false;

		if (!fbxImporter->Import(fbxScene)) return false;

		//Destroy importer as we are done using it
		fbxImporter->Destroy();

		//Create the root node as a handle for the rest of the FBX mesh
		FbxNode* rootNode = fbxScene->GetRootNode();

		//if the root node is not null
		if (rootNode)
		{
			//for every child node
			for (int i = 0; i < rootNode->GetChildCount(); ++i)
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
				int vertCounter = 0;

				for (int j = 0; j < mesh->GetPolygonCount(); ++j)
				{

					int numVerts = mesh->GetPolygonSize(j);

					//if the polgon is not a triangle wether the mesh is not triangulated or some other error
					if (numVerts != 3) return false;

					for (int k = 0; k < numVerts; ++k)
					{
						int iCtrlPoint = mesh->GetPolygonVertex(j, k);

						//if the requested vertex does not exists or the indices arguments have an invalid range
						if (iCtrlPoint < 0) return false;

						Vertex vert;

						//position
						vert.position.x = -(float)verts[iCtrlPoint].mData[0];
						vert.position.y = (float)verts[iCtrlPoint].mData[1];
						vert.position.z = (float)verts[iCtrlPoint].mData[2];

						//uvs
						LoadUV(mesh, iCtrlPoint, mesh->GetTextureUVIndex(j, k), 0, vert);

						//normals
						LoadNormal(mesh, iCtrlPoint, vertCounter, vert);

						// sort so its easier to remove duplicates
						vert.SortBlendingInfoByWeight();

						outVerts->push_back(vert);
						++vertCounter;

					}
				}
			}
			outIndices->clear();
			outIndices->resize(outVerts->size());
			ElimanateDuplicates(*outVerts, *outIndices);
			//swap indices for correct texture
			for (unsigned int i = 0; i < outIndices->size(); i += 3)
			{
				outIndices->at(i + 1) ^= outIndices->at(i + 2);
				outIndices->at(i + 2) ^= outIndices->at(i + 1);
				outIndices->at(i + 1) ^= outIndices->at(i + 2);

			}
			return true;
		}
		return false;

	}

}



