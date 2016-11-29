// FBXLoader.cpp : Defines the exported functions for the DLL application.  
// Compile by using: cl /EHsc /DMATHLIBRARY_EXPORTS /LD MathLibrary.cpp  

#include "FBXLoader.h" 
#include <fbxsdk.h>

FbxManager* fbxManager = nullptr;

namespace FBXLoader
{
	/*----------------------------------------------------------------------------------------------------------------------------------
	-----------------------------------------------------Helper Functions---------------------------------------------------------------
	----------------------------------------------------------------------------------------------------------------------------------*/
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

	void InitWholeSkeleton(FbxMesh* mesh, std::vector<Vertex>& mVertices, std::vector<XMFLOAT4X4> *outBonePos)
	{
		//FbxNode* jointRoot = FindRoot(rootNode, FbxNodeAttribute::eSkeleton);
		unsigned int numOfDeformers = mesh->GetDeformerCount();

		for (unsigned int deformerIndex = 0; deformerIndex < numOfDeformers; ++deformerIndex)
		{
			FbxSkin* currSkin = (FbxSkin*)(mesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));
			if (!currSkin)
			{
				continue;
			}
			unsigned int numBones = currSkin->GetClusterCount();
			for (unsigned int boneIndex = 0; boneIndex < numBones; ++boneIndex)
			{
				FbxCluster* cluster = currSkin->GetCluster(boneIndex);

				//bone ref
				FbxNode* boneNode = cluster->GetLink();

				// Get the bind pose
				FbxAMatrix bindPoseMatrix;
				FbxAMatrix	transformMatrix;
				cluster->GetTransformMatrix(transformMatrix);
				cluster->GetTransformLinkMatrix(bindPoseMatrix);

				// decomposed transform components
				FbxVector4 vS = bindPoseMatrix.GetS();
				FbxVector4 vR = bindPoseMatrix.GetR();
				FbxVector4 vT = bindPoseMatrix.GetT();
				FbxAMatrix fbxGlobalBoneBaseMatrix = cluster->GetLink()->EvaluateGlobalTransform();
				XMFLOAT4X4 mat;
				for (int r = 0; r < 4; r++)
					for (int c = 0; c < 4; c++)
					{
						mat.m[r][c] = (float)fbxGlobalBoneBaseMatrix.mData[r][c];
					}
				outBonePos->push_back(mat);

				int *pVertexIndices = cluster->GetControlPointIndices();
				double *pVertexWeights = cluster->GetControlPointWeights();

				// Iterate through all the vertices, which are affected by the bone
				int ncVertexIndices = cluster->GetControlPointIndicesCount();
				std::vector<int> vertIndVect;
				std::vector<float> weightvect;
				vertIndVect.resize(ncVertexIndices);
				weightvect.resize(ncVertexIndices);

				for (int iBoneVertexIndex = 0; iBoneVertexIndex < ncVertexIndices; iBoneVertexIndex++)
				{
					// vertex
					int niVertex = pVertexIndices[iBoneVertexIndex];
					// weight
					float fWeight = (float)pVertexWeights[iBoneVertexIndex];
					weightvect[iBoneVertexIndex] = fWeight;
					vertIndVect[iBoneVertexIndex] = niVertex;
				}
				for (unsigned i = 0; i < 4; ++i)
				{
					mVertices[vertIndVect[i]].blendingIndices = XMFLOAT4(0, 0, 0, 0);
					mVertices[vertIndVect[i]].blendingWeight = XMFLOAT4(0.25f, 0.25f, 0.25f, 0.25f);
					//mVertices[vertIndVect[i]].blendingWeight = XMFLOAT4(weightvect[0], weightvect[1], weightvect[2], weightvect[3]);
				}
			}

		}
	}

	int FindVertex(const Vertex& inTargetVertex, const std::vector<Vertex>& uniqueVertices)
	{
		for (unsigned int i = 0; i < uniqueVertices.size(); ++i)
			if (inTargetVertex == uniqueVertices[i])
				return i;

		return -1;
	}
	FbxNode* FindRoot(FbxNode* root, FbxNodeAttribute::EType attrib)
	{
		unsigned numChildren = root->GetChildCount();
		if (!numChildren)
			return NULL;

		FbxNode* child = NULL;
		for (unsigned c = 0; c < numChildren; c++)
		{
			child = root->GetChild(c);
			if (!child->GetNodeAttribute())
				continue;
			if (child->GetNodeAttribute()->GetAttributeType() != attrib)
				continue;

			return child;
		}

		FbxNode* rootJoint = NULL;
		for (unsigned c = 0; c < numChildren; c++)
		{
			child = root->GetChild(c);
			rootJoint = FindRoot(child, attrib);
			if (rootJoint)
				break;
		}
		return rootJoint;
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
	/*----------------------------------------------------------------------------------------------------------------------------------
	----------------------------------------------------------------------------------------------------------------------------------*/


	bool Functions::FBXLoadFile(std::vector<Vertex> * outVerts, std::vector<unsigned int> * outIndices, std::vector<XMFLOAT4X4> *outBoneMats, const char * filePath)
	{

		//if the FbxManager is not created. Create it.
		if (!fbxManager)
		{
			fbxManager = FbxManager::Create();

			FbxIOSettings* settings = FbxIOSettings::Create(fbxManager, IOSROOT);
			fbxManager->SetIOSettings(settings);
		}

		FbxMesh* mesh;
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

				mesh = (FbxMesh*)node->GetNodeAttribute();

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

			InitWholeSkeleton(mesh, *outVerts, outBoneMats);
			return true;
		}
		return false;

	}

}



