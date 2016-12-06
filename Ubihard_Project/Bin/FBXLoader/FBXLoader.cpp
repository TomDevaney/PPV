// FBXLoader.cpp : Defines the exported functions for the DLL application.  
// Compile by using: cl /EHsc /DMATHLIBRARY_EXPORTS /LD MathLibrary.cpp  

#include "FBXLoader.h" 
#include <fbxsdk.h>
#include <unordered_map>
#include <algorithm>
#include <fstream>
#include "Animation.h"
#include "KeyFrame.h"
#include "TransformNode.h"

//struct Skeleton
//{
//	std::vector<Joint> mJoints;
//};

namespace FBXLoader
{
	//GLOBALS
	FbxScene* mFBXScene = nullptr;
	FbxManager* mFBXManager = nullptr;
	//Skeleton mSkeleton;
	TomSkeleton tomsSkeleton;
	bool mHasAnimation = true;
	std::vector<Vertex> mVerts;
	std::unordered_map<unsigned int, CtrlPoint*> mControlPoints;
	FbxLongLong mAnimationLength;
	std::string mAnimationName;
	unsigned int transformNodeindex = 0;
	std::vector<KeyFrame> tomKeyFrames;

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

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
					mVertices[vertIndVect[i]].blendingIndices = XMINT4(0, 0, 0, 0);
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

#pragma region Helper_New

	FbxAMatrix GetGeometryTransformation(FbxNode* inNode);
	XMMATRIX FBXToXMMatrix(const FbxAMatrix& inMatrix);

	void ProcessSkeletonHierarchyRecursively(FbxNode* inNode, int inDepth, int myIndex, int inParentIndex, TransformNode* curTransform)
	{
		TransformNode* nextParent = nullptr;
		TransformNode* child = new TransformNode();

		if (inNode->GetNodeAttribute() && inNode->GetNodeAttribute()->GetAttributeType() && inNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
		{
			//Joint currJoint;
			//currJoint.mParentIndex = inParentIndex;
			//currJoint.mName = inNode->GetName();


			FbxAMatrix currentTransformOffset = inNode->EvaluateGlobalTransform(0) *  GetGeometryTransformation(inNode);
			child->world = FBXToXMMatrix(currentTransformOffset.Inverse() * inNode->EvaluateGlobalTransform(0));
			child->name = inNode->GetName();

			if (myIndex == 0) //this means that it's the root node, so I want to make the curTransform = to the curNode
			{
				curTransform = new TransformNode();

				curTransform->world = child->world;
				curTransform->name = child->name;
				curTransform->index = transformNodeindex;
				++transformNodeindex;
				delete child;

				nextParent = curTransform;

				tomsSkeleton.transforms.push_back(curTransform);
				tomsSkeleton.names += curTransform->name;
			}
			else
			{
				child->parent = curTransform;
				child->index = transformNodeindex;
				curTransform->AddChild(child);
				nextParent = child;
				++transformNodeindex;

				tomsSkeleton.transforms.push_back(child);
				tomsSkeleton.names += child->name;
			}


			//mSkeleton->mJoints.push_back(currJoint);
		}
		for (int i = 0; i < inNode->GetChildCount(); i++)
		{
			ProcessSkeletonHierarchyRecursively(inNode->GetChild(i), inDepth + 1, (int)tomsSkeleton.transforms.size(), myIndex, nextParent);

			//ProcessSkeletonHierarchyRecursively(inNode->GetChild(i), inDepth + 1, mSkeleton->mJoints.size(), myIndex, mSkeleton, curTransform);
		}
	}

	void ProcessSkeletonHierarchy(FbxNode* inRootNode)
	{
		TransformNode* root = nullptr;

		//root->name = inRootNode->GetName();

		for (int childIndex = 0; childIndex < inRootNode->GetChildCount(); ++childIndex)
		{
			FbxNode* currNode = inRootNode->GetChild(childIndex);

			//TransformNode* child = new TransformNode();

			//FbxAMatrix currentTransformOffset = currNode->EvaluateGlobalTransform(0) *  GetGeometryTransformation(currNode);
			//child->world = FBXToXMMatrix(currentTransformOffset.Inverse() * currNode->EvaluateGlobalTransform(0));
			//child->name = currNode->GetName();

			//root->AddChild(child);

			ProcessSkeletonHierarchyRecursively(currNode, 0, 0, -1, root);
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
			currPosition.x = -(float)(currMesh->GetControlPointAt(i).mData[0]);
			currPosition.y = (float)(currMesh->GetControlPointAt(i).mData[1]);
			currPosition.z = (float)(currMesh->GetControlPointAt(i).mData[2]);
			currCtrlPoint->mPosition = currPosition;
			mControlPoints[i] = currCtrlPoint;
		}
	}

	XMFLOAT2 ReadUV(FbxMesh* inMesh, int inCtrlPointIndex, int inTextureUVIndex, int inUVLayer)
	{
		XMFLOAT2 outUV;

		if (inUVLayer >= 2 || inMesh->GetElementUVCount() <= inUVLayer) { return outUV; }
		FbxGeometryElementUV* vertexUV = inMesh->GetElementUV(inUVLayer);

		switch (vertexUV->GetMappingMode())
		{
		case FbxGeometryElement::eByControlPoint:
			switch (vertexUV->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				outUV.x = (float)(vertexUV->GetDirectArray().GetAt(inCtrlPointIndex).mData[0]);
				outUV.y = (float)(vertexUV->GetDirectArray().GetAt(inCtrlPointIndex).mData[1]);
				break;
			}

			case FbxGeometryElement::eIndexToDirect:
			{
				int index = vertexUV->GetIndexArray().GetAt(inCtrlPointIndex);
				outUV.x = (float)(vertexUV->GetDirectArray().GetAt(index).mData[0]);
				outUV.y = (float)(vertexUV->GetDirectArray().GetAt(index).mData[1]);
				break;
			}

			default:
				break;
			}

		case FbxGeometryElement::eByPolygonVertex:
			switch (vertexUV->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			case FbxGeometryElement::eIndexToDirect:
			{
				outUV.x = (float)(vertexUV->GetDirectArray().GetAt(inTextureUVIndex).mData[0]);
				outUV.y = (float)(vertexUV->GetDirectArray().GetAt(inTextureUVIndex).mData[1]);
				break;
			}

			default:
				break;
			}
		}
		outUV.y = 1.0f - outUV.y;
		return outUV;
	}

	XMFLOAT3 ReadNormal(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter)
	{
		XMFLOAT3 outNormal;
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
				outNormal.x = (float)(vertexNormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[0]);
				outNormal.y = (float)(vertexNormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[1]);
				outNormal.z = (float)(vertexNormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[2]);
			}
			break;

			case FbxGeometryElement::eIndexToDirect:
			{
				int index = vertexNormal->GetIndexArray().GetAt(inCtrlPointIndex);
				outNormal.x = (float)(vertexNormal->GetDirectArray().GetAt(index).mData[0]);
				outNormal.y = (float)(vertexNormal->GetDirectArray().GetAt(index).mData[1]);
				outNormal.z = (float)(vertexNormal->GetDirectArray().GetAt(index).mData[2]);
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
				outNormal.x = (float)(vertexNormal->GetDirectArray().GetAt(inVertexCounter).mData[0]);
				outNormal.y = (float)(vertexNormal->GetDirectArray().GetAt(inVertexCounter).mData[1]);
				outNormal.z = (float)(vertexNormal->GetDirectArray().GetAt(inVertexCounter).mData[2]);
			}
			break;

			case FbxGeometryElement::eIndexToDirect:
			{
				int index = vertexNormal->GetIndexArray().GetAt(inVertexCounter);
				outNormal.x = (float)(vertexNormal->GetDirectArray().GetAt(index).mData[0]);
				outNormal.y = (float)(vertexNormal->GetDirectArray().GetAt(index).mData[1]);
				outNormal.z = (float)(vertexNormal->GetDirectArray().GetAt(index).mData[2]);
			}
			break;

			default:
				throw std::exception("Invalid Reference");
			}
			break;
		}
		outNormal.x = -outNormal.x;
		return outNormal;
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
		for (unsigned int i = 0; i < tomsSkeleton.transforms.size(); ++i)
		{
			if (tomsSkeleton.transforms[i]->name == inJointName)
			{
				return i;
			}
		}
		return -1;
	}

	XMMATRIX FBXToXMMatrix(const FbxAMatrix& inMatrix)
	{
		return XMMatrixSet(
			(float)(inMatrix.Get(0, 0)), (float)(inMatrix.Get(0, 1)), (float)(inMatrix.Get(0, 2)), (float)(inMatrix.Get(0, 3)),
			(float)(inMatrix.Get(1, 0)), (float)(inMatrix.Get(1, 1)), (float)(inMatrix.Get(1, 2)), (float)(inMatrix.Get(1, 3)),
			(float)(inMatrix.Get(2, 0)), (float)(inMatrix.Get(2, 1)), (float)(inMatrix.Get(2, 2)), (float)(inMatrix.Get(2, 3)),
			(float)(inMatrix.Get(3, 0)), (float)(inMatrix.Get(3, 1)), (float)(inMatrix.Get(3, 2)), (float)(inMatrix.Get(3, 3)));
	}

	void ProcessJointsAndAnimations(FbxNode* inNode)
	{
		FbxMesh* currMesh = inNode->GetMesh();
		unsigned int numOfDeformers = currMesh->GetDeformerCount();

		FbxAMatrix geometryTransform = GetGeometryTransformation(inNode);

		// for each deformer
		for (unsigned int deformerIndex = 0; deformerIndex < numOfDeformers; ++deformerIndex)
		{
			//check if it is a skin
			FbxSkin* currSkin = reinterpret_cast<FbxSkin*>(currMesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));
			if (!currSkin) { continue; }

			//get the animation's info
			FbxAnimStack* currAnimStack = mFBXScene->GetSrcObject<FbxAnimStack>(0);
			FbxString animStackName = currAnimStack->GetName();
			mAnimationName = animStackName.Buffer();
			FbxTakeInfo* takeInfo = mFBXScene->GetTakeInfo(animStackName);
			FbxTime start = takeInfo->mLocalTimeSpan.GetStart();
			FbxTime end = takeInfo->mLocalTimeSpan.GetStop();
			mAnimationLength = end.GetFrameCount(FbxTime::eFrames24) - start.GetFrameCount(FbxTime::eFrames24) + 1;


			//Keyframe** currAnim = &mSkeleton.mJoints[currJointIndex].mAnimation;

			//for each keyframe, loop through all of the bones and get their world matrix at that keyframe
			for (FbxLongLong i = start.GetFrameCount(FbxTime::eFrames24); i <= end.GetFrameCount(FbxTime::eFrames24); ++i)
			{
				//for each cluster
				unsigned int numOfClusters = currSkin->GetClusterCount();
				KeyFrame tomKeyFrame;

				for (unsigned int clusterIndex = 0; clusterIndex < numOfClusters; ++clusterIndex)
				{
					Bone tempBone;
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
					//tomsSkeleton.transforms[currJointIndex]->mGlobalBindposeInverse = FBXToXMMatrix(globalBindposeInverseMatrix);

					//mSkeleton.mJoints[currJointIndex].mGlobalBindposeInverse = FBXToXMMatrix(globalBindposeInverseMatrix);
					//TODO: mSkeleton.mJoints[currJointIndex].mNode = currCluster->GetLink();


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
					//FbxAnimStack* currAnimStack = mFBXScene->GetSrcObject<FbxAnimStack>(0);
					//FbxString animStackName = currAnimStack->GetName();
					//mAnimationName = animStackName.Buffer();
					//FbxTakeInfo* takeInfo = mFBXScene->GetTakeInfo(animStackName);
					//FbxTime start = takeInfo->mLocalTimeSpan.GetStart();
					//FbxTime end = takeInfo->mLocalTimeSpan.GetStop();
					//mAnimationLength = end.GetFrameCount(FbxTime::eFrames24) - start.GetFrameCount(FbxTime::eFrames24) + 1;
					//Keyframe** currAnim = &mSkeleton.mJoints[currJointIndex].mAnimation;


					//for (FbxLongLong i = start.GetFrameCount(FbxTime::eFrames24); i <= end.GetFrameCount(FbxTime::eFrames24); ++i)
					//{
						//set up tombone's world matrix

						//because I added a inverse bind pose, I don't need this
						//FbxTime currTime;
						//currTime.SetFrame(i, FbxTime::eFrames24);
						//FbxAMatrix currentTransformOffset = inNode->EvaluateGlobalTransform(currTime) * geometryTransform;
						//XMFLOAT4X4 world;
						//XMStoreFloat4x4(&world, FBXToXMMatrix(currentTransformOffset.Inverse() * currCluster->GetLink()->EvaluateGlobalTransform(currTime)));
						//tempBone.SetWorld(world);

						//set inverse bind pose of bone
						XMFLOAT4X4 tempBindPoseInverse;
						XMStoreFloat4x4(&tempBindPoseInverse, FBXToXMMatrix(globalBindposeInverseMatrix));
						tempBone.SetInverseBindPose(tempBindPoseInverse);

						//set name of bone
						tempBone.SetName(currJointName);
						

					FbxTime currTime;
					currTime.SetFrame(i, FbxTime::eFrames24);
					//*currAnim = new Keyframe();
					//(*currAnim)->mFrameNum = i;
					//FbxAMatrix currentTransformOffset = inNode->EvaluateGlobalTransform(currTime) * geometryTransform;
					//(*currAnim)->mGlobalTransform = FBXToXMMatrix(currentTransformOffset.Inverse() * currCluster->GetLink()->EvaluateGlobalTransform(currTime));
					//currAnim = &((*currAnim)->mNext);
					//}

						//push back tempBone into current keyframe
						tomKeyFrame.SetTime((float)currTime.GetSecondDouble());
						tomKeyFrame.InsertBone(tempBone);
				}

				//push back current keyframe into vector of all keyframes
				tomKeyFrames.push_back(tomKeyFrame);
			}
		}

		//make transform node using any keyframe

		TransformNode* root = new TransformNode();

		//for (int i = 0; i < 
		//root->AddChild(tomKeyFrames[0].
	}


	void StoreBlendingInfo(Vertex& temp, const std::vector<VertexBlendingInfo>& vertInfos)
	{
		temp.blendingIndices.x = 0;
		temp.blendingIndices.y = 0;
		temp.blendingIndices.z = 0;
		temp.blendingIndices.w = 0;
		temp.blendingWeight.x = 0.0f;
		temp.blendingWeight.y = 0.0f;
		temp.blendingWeight.z = 0.0f;
		temp.blendingWeight.w = 0.0f;

		switch (vertInfos.size())
		{
		default:
			temp.blendingWeight.x = 0.25f;
			temp.blendingWeight.y = 0.25f;
			temp.blendingWeight.z = 0.25f;
			temp.blendingWeight.w = 0.25f;
			break;
		case 1:
			temp.blendingIndices.x = vertInfos[0].mBlendingIndex;
			temp.blendingWeight.x = (float)vertInfos[0].mBlendingWeight;
			break;
		case 2:
			temp.blendingIndices.x = vertInfos[0].mBlendingIndex;
			temp.blendingIndices.y = vertInfos[1].mBlendingIndex;
			temp.blendingWeight.x = (float)vertInfos[0].mBlendingWeight;
			temp.blendingWeight.y = (float)vertInfos[1].mBlendingWeight;
			break;
		case 3:
			temp.blendingIndices.x = vertInfos[0].mBlendingIndex;
			temp.blendingIndices.y = vertInfos[1].mBlendingIndex;
			temp.blendingIndices.z = vertInfos[2].mBlendingIndex;
			temp.blendingWeight.x = (float)vertInfos[0].mBlendingWeight;
			temp.blendingWeight.y = (float)vertInfos[1].mBlendingWeight;
			temp.blendingWeight.z = (float)vertInfos[2].mBlendingWeight;
			break;
		case 4:
			temp.blendingIndices.x = vertInfos[0].mBlendingIndex;
			temp.blendingIndices.y = vertInfos[1].mBlendingIndex;
			temp.blendingIndices.z = vertInfos[2].mBlendingIndex;
			temp.blendingIndices.w = vertInfos[3].mBlendingIndex;
			temp.blendingWeight.x = (float)vertInfos[0].mBlendingWeight;
			temp.blendingWeight.y = (float)vertInfos[1].mBlendingWeight;
			temp.blendingWeight.z = (float)vertInfos[2].mBlendingWeight;
			temp.blendingWeight.w = (float)vertInfos[3].mBlendingWeight;
			break;
		}
	}
	void ProcessMesh(FbxNode* inNode)
	{
		FbxMesh* currMesh = inNode->GetMesh();
		int vertexCounter = 0;
		int ctrlPointIndex = 0;

		// currMesh->GetPolygonCount() == Triangle Count
		for (int i = 0; i < currMesh->GetPolygonCount(); ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				ctrlPointIndex = currMesh->GetPolygonVertex(i, j);
				CtrlPoint* currCtrlPoint = mControlPoints[ctrlPointIndex];


				Vertex temp;
				temp.mPosition = currCtrlPoint->mPosition;
				temp.mNormal = ReadNormal(currMesh, ctrlPointIndex, vertexCounter);
				temp.mUV = ReadUV(currMesh, ctrlPointIndex, currMesh->GetTextureUVIndex(i, j), 0);

				// Copy the blending info from each control point
				std::vector<VertexBlendingInfo> vertInfos;
				for (unsigned int i = 0; i < currCtrlPoint->mBlendingInfo.size(); ++i)
				{
					VertexBlendingInfo currBlendingInfo;
					currBlendingInfo.mBlendingIndex = currCtrlPoint->mBlendingInfo[i].mBlendingIndex;
					currBlendingInfo.mBlendingWeight = currCtrlPoint->mBlendingInfo[i].mBlendingWeight;
					vertInfos.push_back(currBlendingInfo);
				}

				std::sort(vertInfos.begin(), vertInfos.end());
				StoreBlendingInfo(temp, vertInfos);

				mVerts.push_back(temp);
				++vertexCounter;
			}
		}

		// Done using the Control Points so delete them
		for (auto itr = mControlPoints.begin(); itr != mControlPoints.end(); ++itr) { delete itr->second; }
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
					ProcessJointsAndAnimations(inNode);

				ProcessMesh(inNode);
				break;
			}
		}

		//ProcessGeometry on all child nodes 
		for (int i = 0; i < inNode->GetChildCount(); ++i)
			ProcessGeometry(inNode->GetChild(i));



	}



	void ReadBinormal(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter, XMFLOAT3& outBinormal)
	{
		if (inMesh->GetElementBinormalCount() < 1)
		{
			return;
		}

		FbxGeometryElementBinormal* vertexBinormal = inMesh->GetElementBinormal(0);
		switch (vertexBinormal->GetMappingMode())
		{
		case FbxGeometryElement::eByControlPoint:
			switch (vertexBinormal->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				outBinormal.x = (float)(vertexBinormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[0]);
				outBinormal.y = (float)(vertexBinormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[1]);
				outBinormal.z = (float)(vertexBinormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[2]);
				break;
			}

			case FbxGeometryElement::eIndexToDirect:
			{
				int index = vertexBinormal->GetIndexArray().GetAt(inCtrlPointIndex);
				outBinormal.x = (float)(vertexBinormal->GetDirectArray().GetAt(index).mData[0]);
				outBinormal.y = (float)(vertexBinormal->GetDirectArray().GetAt(index).mData[1]);
				outBinormal.z = (float)(vertexBinormal->GetDirectArray().GetAt(index).mData[2]);
				break;
			}

			default:
				break;
			}

		case FbxGeometryElement::eByPolygonVertex:
			switch (vertexBinormal->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				outBinormal.x = (float)(vertexBinormal->GetDirectArray().GetAt(inVertexCounter).mData[0]);
				outBinormal.y = (float)(vertexBinormal->GetDirectArray().GetAt(inVertexCounter).mData[1]);
				outBinormal.z = (float)(vertexBinormal->GetDirectArray().GetAt(inVertexCounter).mData[2]);
				break;
			}

			case FbxGeometryElement::eIndexToDirect:
			{
				int index = vertexBinormal->GetIndexArray().GetAt(inVertexCounter);
				outBinormal.x = (float)(vertexBinormal->GetDirectArray().GetAt(index).mData[0]);
				outBinormal.y = (float)(vertexBinormal->GetDirectArray().GetAt(index).mData[1]);
				outBinormal.z = (float)(vertexBinormal->GetDirectArray().GetAt(index).mData[2]);
				break;
			}

			default:
				break;
			}
		}
	}

	void ReadTangent(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter, XMFLOAT3& outTangent)
	{
		if (inMesh->GetElementTangentCount() < 1)
		{
			return;
		}

		FbxGeometryElementTangent* vertexTangent = inMesh->GetElementTangent(0);
		switch (vertexTangent->GetMappingMode())
		{
		case FbxGeometryElement::eByControlPoint:
			switch (vertexTangent->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				outTangent.x = (float)(vertexTangent->GetDirectArray().GetAt(inCtrlPointIndex).mData[0]);
				outTangent.y = (float)(vertexTangent->GetDirectArray().GetAt(inCtrlPointIndex).mData[1]);
				outTangent.z = (float)(vertexTangent->GetDirectArray().GetAt(inCtrlPointIndex).mData[2]);
				break;
			}

			case FbxGeometryElement::eIndexToDirect:
			{
				int index = vertexTangent->GetIndexArray().GetAt(inCtrlPointIndex);
				outTangent.x = (float)(vertexTangent->GetDirectArray().GetAt(index).mData[0]);
				outTangent.y = (float)(vertexTangent->GetDirectArray().GetAt(index).mData[1]);
				outTangent.z = (float)(vertexTangent->GetDirectArray().GetAt(index).mData[2]);
				break;
			}

			default:
				break;
			}

		case FbxGeometryElement::eByPolygonVertex:
			switch (vertexTangent->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				outTangent.x = (float)(vertexTangent->GetDirectArray().GetAt(inVertexCounter).mData[0]);
				outTangent.y = (float)(vertexTangent->GetDirectArray().GetAt(inVertexCounter).mData[1]);
				outTangent.z = (float)(vertexTangent->GetDirectArray().GetAt(inVertexCounter).mData[2]);
				break;
			}

			case FbxGeometryElement::eIndexToDirect:
			{
				int index = vertexTangent->GetIndexArray().GetAt(inVertexCounter);
				outTangent.x = (float)(vertexTangent->GetDirectArray().GetAt(index).mData[0]);
				outTangent.y = (float)(vertexTangent->GetDirectArray().GetAt(index).mData[1]);
				outTangent.z = (float)(vertexTangent->GetDirectArray().GetAt(index).mData[2]);
				break;
			}

			default:
				break;
			}
		}
	}

	void CleanupFBX()
	{
		mFBXScene->Destroy();
		mFBXManager->Destroy();
		mFBXManager = nullptr;
		mFBXScene = nullptr;

		mVerts.clear();
		//mSkeleton.mJoints.clear();
		tomsSkeleton.names.clear();
		tomsSkeleton.transforms.clear();
		tomKeyFrames.clear();
	}
#pragma endregion 
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

						vert.blendingIndices = XMINT4(0, 0, 0, 0);
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

	std::vector<FriendlyIOTransformNode> friendlyNodes;

	void MakeFriendlyNodeRecursive(TransformNode* tNode)
	{
		FriendlyIOTransformNode friendlyNode;

		if (tNode)
		{
			friendlyNode.parentIndex = tNode->parent->index;
			friendlyNode.nameOffset = friendlyNodes[friendlyNode.parentIndex].nameOffset + (unsigned int)tNode->name.size();
			DirectX::XMStoreFloat4x4(&friendlyNode.world, tNode->world);
			//friendlyNode.world = tNode->world;

			if (tNode->child)
			{
				friendlyNode.childIndex = tNode->child->index;
			}
			else
			{
				friendlyNode.childIndex = -1;
			}

			if (tNode->sibling)
			{
				friendlyNode.siblingIndex = tNode->sibling->index;
			}
			else
			{
				friendlyNode.siblingIndex = -1;
			}

			friendlyNodes.push_back(friendlyNode);

			MakeFriendlyNodeRecursive(tNode->child);
			MakeFriendlyNodeRecursive(tNode->sibling);
		}
	}

	void MakeFriendlyNode(TransformNode* tNode)
	{
		FriendlyIOTransformNode friendlyNode;

		friendlyNode.parentIndex = -1;
		friendlyNode.childIndex = tNode->child->index;
		friendlyNode.siblingIndex = -1;
		friendlyNode.nameOffset = 0;
		//friendlyNode.name = tNode->name;
		DirectX::XMStoreFloat4x4(&friendlyNode.world, tNode->world);

		friendlyNodes.push_back(friendlyNode);

		MakeFriendlyNodeRecursive(tNode->child);
		//MakeFriendlyNodeRecursive(tNode->sibling);
	}

	void ExportToBinary(const char * filePath)
	{
		std::ofstream bout;
		std::string path;
		unsigned int numBones = 0, namesSize = 0;

		path = "../Resources/Box/";
		path += "Box.skel";
		//path += filePath

		bout.open(path, std::ios::binary); //will truncate existing file

		if (bout.is_open())
		{
			//get length of bones
			numBones = (unsigned int)tomsSkeleton.transforms.size();
			namesSize = (unsigned int)tomsSkeleton.names.size();

			//write header
			bout.write((const char*)&numBones, sizeof(unsigned int));
			bout.write((const char*)&namesSize, sizeof(unsigned int));

			//make transform nodes that are friendly
			//FriendlyIOTransformNode friendlyNode;

			MakeFriendlyNode(tomsSkeleton.transforms[0]);

			//write out transform data
			//for (int i = 0; i < friendlyNodes.size(); ++i)
			{
				bout.write((const char*)friendlyNodes.data(), sizeof(FriendlyIOTransformNode) * friendlyNodes.size());
			}

			//write out names
			bout.write((const char*)tomsSkeleton.names.data(), namesSize);
		}
		
		bout.close();

		//now animation file7
		path = "../Resources/Box/";
		path += "Box_Idle.anim";

		bout.open(path, std::ios::binary);

		if (bout.is_open())
		{
			//make animation
			//Animation anim;
			//anim.Init(AnimType::LOOP, tomKeyFrames[tomKeyFrames.size() - 1].GetTime() - tomKeyFrames[0].GetTime(), tomKeyFrames);
			AnimType type;

			type = AnimType::LOOP;

			//write out header info
			unsigned int numOfKeyFrames;
			unsigned int numOfBones;

			numOfKeyFrames = (unsigned int)tomKeyFrames.size();
			numOfBones = (unsigned int)tomKeyFrames[0].GetBones().size();

			bout.write((const char*)&numOfKeyFrames, sizeof(unsigned int));
			bout.write((const char*)&numOfBones, sizeof(unsigned int));

			//write out keyframes
			for (int i = 0; i < tomKeyFrames.size(); ++i)
			{
				float keyFrameTime = tomKeyFrames[i].GetTime();
				bout.write((const char*)tomKeyFrames[i].GetBones().data(), sizeof(Bone) * numOfBones);
				bout.write((const char*)&keyFrameTime, sizeof(float));
			}

			//write out animtype
			bout.write((const char*)&type, sizeof(AnimType));

			//write out time
			float time;
			time = tomKeyFrames[tomKeyFrames.size() - 1].GetTime() - tomKeyFrames[0].GetTime();

			bout.write((const char*)&time, sizeof(float));

			bout.close();
		}
	}

	FBXLOADER_API bool Functions::FBXExportToBinary(std::vector<Vertex>* outVerts, std::vector<unsigned int>* outIndices, const char * inFilePath, const char * outFilePath)
	{

		//if the FbxManager is not created. Create it.
		if (!mFBXManager)
		{
			mFBXManager = FbxManager::Create();

			FbxIOSettings* settings = FbxIOSettings::Create(mFBXManager, IOSROOT);
			mFBXManager->SetIOSettings(settings);
		}

		FbxImporter* fbxImporter = FbxImporter::Create(mFBXManager, "");
		mFBXScene = FbxScene::Create(mFBXManager, "");

		// the -1 is so that the plugin will detect the file format according to file suffix automatically.
		if (!fbxImporter->Initialize(inFilePath, -1, mFBXManager->GetIOSettings())) return false;

		if (!fbxImporter->Import(mFBXScene)) return false;

		//Destroy importer as we are done using it
		fbxImporter->Destroy();

		//Create the root node as a handle for the rest of the FBX mesh
		FbxNode* rootNode = mFBXScene->GetRootNode();

		//clear skeleton
		tomsSkeleton.transforms.clear();

		//if the root node is not null
		if (rootNode)
		{

			// Get the clean name of the model
			ProcessSkeletonHierarchy(mFBXScene->GetRootNode());
			if (tomsSkeleton.transforms.empty()) { mHasAnimation = false; }

			//if (mSkeleton.mJoints.empty()) { mHasAnimation = false; }


			ProcessGeometry(mFBXScene->GetRootNode());

			outIndices->clear();
			outIndices->resize(mVerts.size());
			ElimanateDuplicates(mVerts, *outIndices);

			//swap indices for correct texture
			for (unsigned int i = 0; i < outIndices->size(); i += 3)
			{
				outIndices->at(i + 1) ^= outIndices->at(i + 2);
				outIndices->at(i + 2) ^= outIndices->at(i + 1);
				outIndices->at(i + 1) ^= outIndices->at(i + 2);
			}
			*outVerts = mVerts;

			ExportToBinary(rootNode->GetName());

			CleanupFBX();
			return true;
		}
		return false;

	}

}










