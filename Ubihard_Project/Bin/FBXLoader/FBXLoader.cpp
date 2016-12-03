// FBXLoader.cpp : Defines the exported functions for the DLL application.  
// Compile by using: cl /EHsc /DMATHLIBRARY_EXPORTS /LD MathLibrary.cpp  

#include "FBXLoader.h" 
#include <fbxsdk.h>
#include <unordered_map>

FbxManager* mFBXManager = nullptr;
Skeleton mSkeleton;
bool mHasAnimation = true;
std::vector<Vertex> mVerts;
std::unordered_map<unsigned int, CtrlPoint*> mControlPoints;

namespace FBXLoader
{
	//GLOBALS
	FbxScene* mFBXScene = nullptr;

	/*----------------------------------------------------------------------------------------------------------------------------------
	-----------------------------------------------------Helper Functions---------------------------------------------------------------
	----------------------------------------------------------------------------------------------------------------------------------*/
#pragma region Helper_Functions  
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
				iVert.mUV.x = (float)(vertUV->GetDirectArray().GetAt(iCtrlPoint).mData[0]);
				iVert.mUV.y = (float)(vertUV->GetDirectArray().GetAt(iCtrlPoint).mData[1]);
			}
			break;

			case FbxGeometryElement::eIndexToDirect:
			{
				int index = vertUV->GetIndexArray().GetAt(iCtrlPoint);
				iVert.mUV.x = (float)(vertUV->GetDirectArray().GetAt(index).mData[0]);
				iVert.mUV.y = (float)(vertUV->GetDirectArray().GetAt(index).mData[1]);
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
				iVert.mUV.x = (float)(vertUV->GetDirectArray().GetAt(iTextureUVIndex).mData[0]);
				iVert.mUV.y = (float)(vertUV->GetDirectArray().GetAt(iTextureUVIndex).mData[1]);
			}
			break;

			default:
				break;
			}
		}
		iVert.mUV.y = 1.0f - iVert.mUV.y;
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
				vert.mNormal.x = (float)(vertNormal->GetDirectArray().GetAt(iCtrlPoint).mData[0]);
				vert.mNormal.y = (float)(vertNormal->GetDirectArray().GetAt(iCtrlPoint).mData[1]);
				vert.mNormal.z = (float)(vertNormal->GetDirectArray().GetAt(iCtrlPoint).mData[2]);
			}
			break;

			case FbxGeometryElement::eIndexToDirect:
			{
				int index = vertNormal->GetIndexArray().GetAt(iCtrlPoint);
				vert.mNormal.x = (float)(vertNormal->GetDirectArray().GetAt(index).mData[0]);
				vert.mNormal.y = (float)(vertNormal->GetDirectArray().GetAt(index).mData[1]);
				vert.mNormal.z = (float)(vertNormal->GetDirectArray().GetAt(index).mData[2]);
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
				vert.mNormal.x = (float)(vertNormal->GetDirectArray().GetAt(iVertCounter).mData[0]);
				vert.mNormal.y = (float)(vertNormal->GetDirectArray().GetAt(iVertCounter).mData[1]);
				vert.mNormal.z = (float)(vertNormal->GetDirectArray().GetAt(iVertCounter).mData[2]);
			}
			break;

			case FbxGeometryElement::eIndexToDirect:
			{
				int index = vertNormal->GetIndexArray().GetAt(iVertCounter);
				vert.mNormal.x = (float)(vertNormal->GetDirectArray().GetAt(index).mData[0]);
				vert.mNormal.y = (float)(vertNormal->GetDirectArray().GetAt(index).mData[1]);
				vert.mNormal.z = (float)(vertNormal->GetDirectArray().GetAt(index).mData[2]);
			}
			break;

			default:
				break;
			}
		}
		vert.mNormal.x = -vert.mNormal.x;
	}

	void InitWholeSkeleton(FbxMesh* mesh, std::vector<Vertex>& mVertices, std::vector<DirectX::XMFLOAT4X4> *outBonePos)
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
				DirectX::XMFLOAT4X4 mat;
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
	int FindVertex(const VS_BasicInput& inTargetVertex, const std::vector<VS_BasicInput>& uniqueVertices)
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
	void ElimanateDuplicates(std::vector<VS_BasicInput>& mVertices, std::vector<unsigned int>& mIndices)
	{
		// First get a list of unique vertices
		std::vector<VS_BasicInput> uniqueVertices;
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
#pragma endregion   


	void ProcessSkeletonHierarchyRecursively(FbxNode* inNode, int inDepth, int myIndex, int inParentIndex, Skeleton * mSkeleton)
	{
		if (inNode->GetNodeAttribute() && inNode->GetNodeAttribute()->GetAttributeType() && inNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
		{
			Joint currJoint;
			currJoint.mParentIndex = inParentIndex;
			currJoint.mName = inNode->GetName();
			mSkeleton->mJoints.push_back(currJoint);
		}
		for (int i = 0; i < inNode->GetChildCount(); i++)
		{
			ProcessSkeletonHierarchyRecursively(inNode->GetChild(i), inDepth + 1, mSkeleton->mJoints.size(), myIndex, mSkeleton);
		}
	}

	void ProcessSkeletonHierarchy(FbxNode* inRootNode, Skeleton* mSkeleton)
	{

		for (int childIndex = 0; childIndex < inRootNode->GetChildCount(); ++childIndex)
		{
			FbxNode* currNode = inRootNode->GetChild(childIndex);
			ProcessSkeletonHierarchyRecursively(currNode, 0, 0, -1, mSkeleton);
		}
	}

	void ProcessControlPoints(FbxNode* inNode)
	{
		FbxMesh* currMesh = inNode->GetMesh();
		unsigned int ctrlPointCount = currMesh->GetControlPointsCount();
		for (unsigned int i = 0; i < ctrlPointCount; ++i)
		{
			CtrlPoint* currCtrlPoint = new CtrlPoint();
			XMFLOAT3 currPosition;
			currPosition.x = static_cast<float>(currMesh->GetControlPointAt(i).mData[0]);
			currPosition.y = static_cast<float>(currMesh->GetControlPointAt(i).mData[1]);
			currPosition.z = static_cast<float>(currMesh->GetControlPointAt(i).mData[2]);
			currCtrlPoint->mPosition = currPosition;
			mControlPoints[i] = currCtrlPoint;
		}
	}

	void ReadUV(FbxMesh* inMesh, int inCtrlPointIndex, int inTextureUVIndex, int inUVLayer, XMFLOAT2& outUV)
	{
		if (inUVLayer >= 2 || inMesh->GetElementUVCount() <= inUVLayer)
		{
			throw std::exception("Invalid UV Layer Number");
		}
		FbxGeometryElementUV* vertexUV = inMesh->GetElementUV(inUVLayer);

		switch (vertexUV->GetMappingMode())
		{
		case FbxGeometryElement::eByControlPoint:
			switch (vertexUV->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				outUV.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(inCtrlPointIndex).mData[0]);
				outUV.y = static_cast<float>(vertexUV->GetDirectArray().GetAt(inCtrlPointIndex).mData[1]);
			}
			break;

			case FbxGeometryElement::eIndexToDirect:
			{
				int index = vertexUV->GetIndexArray().GetAt(inCtrlPointIndex);
				outUV.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(index).mData[0]);
				outUV.y = static_cast<float>(vertexUV->GetDirectArray().GetAt(index).mData[1]);
			}
			break;

			default:
				throw std::exception("Invalid Reference");
			}
			break;

		case FbxGeometryElement::eByPolygonVertex:
			switch (vertexUV->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			case FbxGeometryElement::eIndexToDirect:
			{
				outUV.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(inTextureUVIndex).mData[0]);
				outUV.y = static_cast<float>(vertexUV->GetDirectArray().GetAt(inTextureUVIndex).mData[1]);
			}
			break;

			default:
				throw std::exception("Invalid Reference");
			}
			break;
		}
	}

	void ReadNormal(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter, XMFLOAT3& outNormal)
	{
		if (inMesh->GetElementNormalCount() < 1)
		{
			throw std::exception("Invalid Normal Number");
		}

		FbxGeometryElementNormal* vertexNormal = inMesh->GetElementNormal(0);
		switch (vertexNormal->GetMappingMode())
		{
		case FbxGeometryElement::eByControlPoint:
			switch (vertexNormal->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[0]);
				outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[1]);
				outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[2]);
			}
			break;

			case FbxGeometryElement::eIndexToDirect:
			{
				int index = vertexNormal->GetIndexArray().GetAt(inCtrlPointIndex);
				outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[0]);
				outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[1]);
				outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[2]);
			}
			break;

			default:
				throw std::exception("Invalid Reference");
			}
			break;

		case FbxGeometryElement::eByPolygonVertex:
			switch (vertexNormal->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inVertexCounter).mData[0]);
				outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inVertexCounter).mData[1]);
				outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inVertexCounter).mData[2]);
			}
			break;

			case FbxGeometryElement::eIndexToDirect:
			{
				int index = vertexNormal->GetIndexArray().GetAt(inVertexCounter);
				outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[0]);
				outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[1]);
				outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[2]);
			}
			break;

			default:
				throw std::exception("Invalid Reference");
			}
			break;
		}
	}


	FbxAMatrix GetGeometryTransformation(FbxNode* inNode)
	{
		const FbxVector4 lT = inNode->GetGeometricTranslation(FbxNode::eSourcePivot);
		const FbxVector4 lR = inNode->GetGeometricRotation(FbxNode::eSourcePivot);
		const FbxVector4 lS = inNode->GetGeometricScaling(FbxNode::eSourcePivot);

		return FbxAMatrix(lT, lR, lS);
	}

	unsigned int FindJointIndexUsingName(const std::string& inJointName)
	{
		for (unsigned int i = 0; i < mSkeleton.mJoints.size(); ++i)
		{
			if (mSkeleton.mJoints[i].mName == inJointName)
			{
				return i;
			}
		}
	}

	void ProcessJointsAndAnimations(FbxNode* inNode)
	{
		FbxMesh* currMesh = inNode->GetMesh();
		unsigned int numOfDeformers = currMesh->GetDeformerCount();
		// This geometry transform is something I cannot understand
		// I think it is from MotionBuilder
		// If you are using Maya for your models, 99% this is just an
		// identity matrix
		// But I am taking it into account anyways......
		FbxAMatrix geometryTransform = GetGeometryTransformation(inNode);

		// A deformer is a FBX thing, which contains some clusters
		// A cluster contains a link, which is basically a joint
		// Normally, there is only one deformer in a mesh
		for (unsigned int deformerIndex = 0; deformerIndex < numOfDeformers; ++deformerIndex)
		{
			// There are many types of deformers in Maya,
			// We are using only skins, so we see if this is a skin
			FbxSkin* currSkin = reinterpret_cast<FbxSkin*>(currMesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));
			if (!currSkin) { continue; }

			unsigned int numOfClusters = currSkin->GetClusterCount();
			for (unsigned int clusterIndex = 0; clusterIndex < numOfClusters; ++clusterIndex)
			{
				FbxCluster* currCluster = currSkin->GetCluster(clusterIndex);
				std::string currJointName = currCluster->GetLink()->GetName();
				unsigned int currJointIndex = FindJointIndexUsingName(currJointName);
				FbxAMatrix transformMatrix;
				FbxAMatrix transformLinkMatrix;
				FbxAMatrix globalBindposeInverseMatrix;

				currCluster->GetTransformMatrix(transformMatrix);	// The transformation of the mesh at binding time
				currCluster->GetTransformLinkMatrix(transformLinkMatrix);	// The transformation of the cluster(joint) at binding time from joint space to world space
				globalBindposeInverseMatrix = transformLinkMatrix.Inverse() * transformMatrix * geometryTransform;

				// Update the information in mSkeleton 
				mSkeleton.mJoints[currJointIndex].mGlobalBindposeInverse = XMMatrixIdentity(); //TODO: //globalBindposeInverseMatrix;

				// Associate each joint with the control points it affects
				unsigned int numOfIndices = currCluster->GetControlPointIndicesCount();
				for (unsigned int i = 0; i < numOfIndices; ++i)
				{
					VertexBlendingInfo currBlendingIndexWeightPair;
					currBlendingIndexWeightPair.mBlendingIndex = currJointIndex;
					currBlendingIndexWeightPair.mBlendingWeight = currCluster->GetControlPointWeights()[i];
					mControlPoints[currCluster->GetControlPointIndices()[i]]->mBlendingInfo.push_back(currBlendingIndexWeightPair);
				}

				// Get animation information
				// Now only supports one take
				FbxAnimStack* currAnimStack = mFBXScene->GetSrcObject<FbxAnimStack>(0);
				FbxString animStackName = currAnimStack->GetName();
				//mAnimationName = animStackName.Buffer();
				FbxTakeInfo* takeInfo = mFBXScene->GetTakeInfo(animStackName);
				FbxTime start = takeInfo->mLocalTimeSpan.GetStart();
				FbxTime end = takeInfo->mLocalTimeSpan.GetStop();
				//mAnimationLength = end.GetFrameCount(FbxTime::eFrames24) - start.GetFrameCount(FbxTime::eFrames24) + 1;
				Keyframe** currAnim = &mSkeleton.mJoints[currJointIndex].mAnimation;

				for (FbxLongLong i = start.GetFrameCount(FbxTime::eFrames24); i <= end.GetFrameCount(FbxTime::eFrames24); ++i)
				{
					FbxTime currTime;
					currTime.SetFrame(i, FbxTime::eFrames24);
					*currAnim = new Keyframe();
					(*currAnim)->mFrameNum = i;
					FbxAMatrix currentTransformOffset = inNode->EvaluateGlobalTransform(currTime) * geometryTransform;
					//TODO: (*currAnim)->mGlobalTransform = currentTransformOffset.Inverse() * currCluster->GetLink()->EvaluateGlobalTransform(currTime);
					currAnim = &((*currAnim)->mNext);
				}
			}
		}

		// Some of the control points only have less than 4 joints
		// affecting them.
		// For a normal renderer, there are usually 4 joints
		// I am adding more dummy joints if there isn't enough
		VertexBlendingInfo currBlendingIndexWeightPair;
		currBlendingIndexWeightPair.mBlendingIndex = 0;
		currBlendingIndexWeightPair.mBlendingWeight = 0;
		for (auto itr = mControlPoints.begin(); itr != mControlPoints.end(); ++itr)
		{
			for (unsigned int i = itr->second->mBlendingInfo.size(); i <= 4; ++i)
			{
				itr->second->mBlendingInfo.push_back(currBlendingIndexWeightPair);
			}
		}
	}

	void ProcessMesh(FbxNode* inNode)
	{
		FbxMesh* currMesh = inNode->GetMesh();
		int mTriangleCount = currMesh->GetPolygonCount();
		int vertexCounter = 0;

		for (unsigned int i = 0; i < mTriangleCount; ++i)
		{
			XMFLOAT3 normal[3];
			XMFLOAT3 tangent[3];
			XMFLOAT3 binormal[3];
			XMFLOAT2 UV[3][2];

			for (unsigned int j = 0; j < 3; ++j)
			{
				int ctrlPointIndex = currMesh->GetPolygonVertex(i, j);
				CtrlPoint* currCtrlPoint = mControlPoints[ctrlPointIndex];


				ReadNormal(currMesh, ctrlPointIndex, vertexCounter, normal[j]);
				// We only have diffuse texture
				for (int k = 0; k < 1; ++k)
				{
					ReadUV(currMesh, ctrlPointIndex, currMesh->GetTextureUVIndex(i, j), k, UV[j][k]);
				}


				Vertex temp;
				temp.mPosition = currCtrlPoint->mPosition;
				temp.mNormal = normal[j];
				temp.mUV = UV[j][0];
				// Copy the blending info from each control point
				for (unsigned int i = 0; i < currCtrlPoint->mBlendingInfo.size(); ++i)
				{
					VertexBlendingInfo currBlendingInfo;
					currBlendingInfo.mBlendingIndex = currCtrlPoint->mBlendingInfo[i].mBlendingIndex;
					currBlendingInfo.mBlendingWeight = currCtrlPoint->mBlendingInfo[i].mBlendingWeight;
					//TODO: temp.mVertexBlendingInfos.push_back(currBlendingInfo);
				}
				// Sort the blending info so that later we can remove
				// duplicated vertices
				//TODO: temp.SortBlendingInfoByWeight();

				mVerts.push_back(temp);
				++vertexCounter;
			}
		}

		// Now mControlPoints has served its purpose
		// We can free its memory
		for (auto itr = mControlPoints.begin(); itr != mControlPoints.end(); ++itr)
		{
			delete itr->second;
		}
		mControlPoints.clear();
	}

	void ProcessGeometry(FbxNode* inNode)
	{
		if (inNode->GetNodeAttribute())
		{
			switch (inNode->GetNodeAttribute()->GetAttributeType())
			{
			case FbxNodeAttribute::eMesh:
				ProcessControlPoints(inNode);
				if (mHasAnimation)
				{
					ProcessJointsAndAnimations(inNode);
				}
				ProcessMesh(inNode);
				break;
			}
		}

		for (int i = 0; i < inNode->GetChildCount(); ++i)
		{
			ProcessGeometry(inNode->GetChild(i));
		}
	}



	void ReadBinormal(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter, XMFLOAT3& outBinormal)
	{
		if (inMesh->GetElementBinormalCount() < 1)
		{
			throw std::exception("Invalid Binormal Number");
		}

		FbxGeometryElementBinormal* vertexBinormal = inMesh->GetElementBinormal(0);
		switch (vertexBinormal->GetMappingMode())
		{
		case FbxGeometryElement::eByControlPoint:
			switch (vertexBinormal->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[0]);
				outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[1]);
				outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[2]);
			}
			break;

			case FbxGeometryElement::eIndexToDirect:
			{
				int index = vertexBinormal->GetIndexArray().GetAt(inCtrlPointIndex);
				outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[0]);
				outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[1]);
				outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[2]);
			}
			break;

			default:
				throw std::exception("Invalid Reference");
			}
			break;

		case FbxGeometryElement::eByPolygonVertex:
			switch (vertexBinormal->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inVertexCounter).mData[0]);
				outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inVertexCounter).mData[1]);
				outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inVertexCounter).mData[2]);
			}
			break;

			case FbxGeometryElement::eIndexToDirect:
			{
				int index = vertexBinormal->GetIndexArray().GetAt(inVertexCounter);
				outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[0]);
				outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[1]);
				outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[2]);
			}
			break;

			default:
				throw std::exception("Invalid Reference");
			}
			break;
		}
	}

	void ReadTangent(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter, XMFLOAT3& outTangent)
	{
		if (inMesh->GetElementTangentCount() < 1)
		{
			throw std::exception("Invalid Tangent Number");
		}

		FbxGeometryElementTangent* vertexTangent = inMesh->GetElementTangent(0);
		switch (vertexTangent->GetMappingMode())
		{
		case FbxGeometryElement::eByControlPoint:
			switch (vertexTangent->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				outTangent.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inCtrlPointIndex).mData[0]);
				outTangent.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inCtrlPointIndex).mData[1]);
				outTangent.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inCtrlPointIndex).mData[2]);
			}
			break;

			case FbxGeometryElement::eIndexToDirect:
			{
				int index = vertexTangent->GetIndexArray().GetAt(inCtrlPointIndex);
				outTangent.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[0]);
				outTangent.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[1]);
				outTangent.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[2]);
			}
			break;

			default:
				throw std::exception("Invalid Reference");
			}
			break;

		case FbxGeometryElement::eByPolygonVertex:
			switch (vertexTangent->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				outTangent.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inVertexCounter).mData[0]);
				outTangent.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inVertexCounter).mData[1]);
				outTangent.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inVertexCounter).mData[2]);
			}
			break;

			case FbxGeometryElement::eIndexToDirect:
			{
				int index = vertexTangent->GetIndexArray().GetAt(inVertexCounter);
				outTangent.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[0]);
				outTangent.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[1]);
				outTangent.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[2]);
			}
			break;

			default:
				throw std::exception("Invalid Reference");
			}
			break;
		}
	}

	void CleanupFbxManager()
	{
		mFBXScene->Destroy();
		mFBXManager->Destroy();


		mVerts.clear();

		mSkeleton.mJoints.clear();

	}

	/*----------------------------------------------------------------------------------------------------------------------------------
	----------------------------------------------------------------------------------------------------------------------------------*/

	FBXLOADER_API bool Functions::FBXLoadFileBasic(std::vector<VS_BasicInput>* outVerts, std::vector<unsigned int>* outIndices, const char * filePath)
	{
		//if the FbxManager is not created. Create it.
		if (!mFBXManager)
		{
			mFBXManager = FbxManager::Create();

			FbxIOSettings* settings = FbxIOSettings::Create(mFBXManager, IOSROOT);
			mFBXManager->SetIOSettings(settings);
		}

		FbxMesh* mesh;
		FbxImporter* fbxImporter = FbxImporter::Create(mFBXManager, "");
		FbxScene* fbxScene = FbxScene::Create(mFBXManager, "");

		// the -1 is so that the plugin will detect the file format according to file suffix automatically.
		if (!fbxImporter->Initialize(filePath, -1, mFBXManager->GetIOSettings())) return false;

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

						VS_BasicInput vert;

						//position
						vert.position.x = -(float)verts[iCtrlPoint].mData[0];
						vert.position.y = (float)verts[iCtrlPoint].mData[1];
						vert.position.z = (float)verts[iCtrlPoint].mData[2];


						outVerts->push_back(vert);

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

	FBXLOADER_API bool Functions::FBXLoadFile(std::vector<Vertex> * outVerts, std::vector<unsigned int> * outIndices, std::vector<DirectX::XMFLOAT4X4> *outBoneMats, const char * filePath)
	{

		//if the FbxManager is not created. Create it.
		if (!mFBXManager)
		{
			mFBXManager = FbxManager::Create();

			FbxIOSettings* settings = FbxIOSettings::Create(mFBXManager, IOSROOT);
			mFBXManager->SetIOSettings(settings);
		}

		FbxMesh* mesh;
		FbxImporter* fbxImporter = FbxImporter::Create(mFBXManager, "");
		FbxScene* fbxScene = FbxScene::Create(mFBXManager, "");

		// the -1 is so that the plugin will detect the file format according to file suffix automatically.
		if (!fbxImporter->Initialize(filePath, -1, mFBXManager->GetIOSettings())) return false;

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
						vert.mPosition.x = -(float)verts[iCtrlPoint].mData[0];
						vert.mPosition.y = (float)verts[iCtrlPoint].mData[1];
						vert.mPosition.z = (float)verts[iCtrlPoint].mData[2];

						//uvs
						LoadUV(mesh, iCtrlPoint, mesh->GetTextureUVIndex(j, k), 0, vert);

						//normals
						LoadNormal(mesh, iCtrlPoint, vertCounter, vert);

						// sort so its easier to remove duplicates

						vert.blendingIndices = XMFLOAT4(0, 0, 0, 0);
						vert.blendingWeight = XMFLOAT4(0.25f, 0.25f, 0.25f, 0.25f);

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


	FBXLOADER_API bool Functions::FBXLoadFile(std::vector<Vertex>* outVerts, std::vector<unsigned int>* outIndices, const char * filePath)
	{

		//if the FbxManager is not created. Create it.
		if (!mFBXManager)
		{
			mFBXManager = FbxManager::Create();

			FbxIOSettings* settings = FbxIOSettings::Create(mFBXManager, IOSROOT);
			mFBXManager->SetIOSettings(settings);
		}

		FbxMesh* mesh;
		FbxImporter* fbxImporter = FbxImporter::Create(mFBXManager, "");
		mFBXScene = FbxScene::Create(mFBXManager, "");

		// the -1 is so that the plugin will detect the file format according to file suffix automatically.
		if (!fbxImporter->Initialize(filePath, -1, mFBXManager->GetIOSettings())) return false;

		if (!fbxImporter->Import(mFBXScene)) return false;

		//Destroy importer as we are done using it
		fbxImporter->Destroy();

		//Create the root node as a handle for the rest of the FBX mesh
		FbxNode* rootNode = mFBXScene->GetRootNode();

		//if the root node is not null
		if (rootNode)
		{

			// Get the clean name of the model
			ProcessSkeletonHierarchy(mFBXScene->GetRootNode(), &mSkeleton);
			if (mSkeleton.mJoints.empty()) { mHasAnimation = false; }


			ProcessGeometry(mFBXScene->GetRootNode());

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
			CleanupFbxManager();
			return true;
		}
		return false;

	}

}










