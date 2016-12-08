// FBXLoader.cpp : Defines the exported functions for the DLL application.  

#include "FBXLoader.h" 
#include <fbxsdk.h>
#include <unordered_map>
#include <algorithm>
#include <fstream>
#include "Animation.h"
#include "KeyFrame.h"
#include "TransformNode.h"

//DEFINES
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

namespace FBXLoader
{
	/*----------------------------------------------------------------------------------------------------------------------------------
	-----------------------------------------------------Global Variables---------------------------------------------------------------
	----------------------------------------------------------------------------------------------------------------------------------*/
#pragma region Globals
	FbxScene* mFBXScene = nullptr;
	FbxManager* mFBXManager = nullptr;

	bool mHasAnimation = true;
	bool isBasic = false;
	unsigned int transformNodeindex = 0;

	std::vector<Vertex> mVerts;
	std::vector<VS_BasicInput> mBasicVerts;
	std::vector<unsigned int> mIndices;
	std::vector<KeyFrame> tomKeyFrames;
	std::vector<FriendlyIOTransformNode> friendlyNodes;
	std::unordered_map<unsigned int, CtrlPoint*> mControlPoints;
	TomSkeleton tomsSkeleton;
#pragma endregion
	/*----------------------------------------------------------------------------------------------------------------------------------
	----------------------------------------------------------------------------------------------------------------------------------*/

	/*----------------------------------------------------------------------------------------------------------------------------------
	-----------------------------------------------------Helper Functions---------------------------------------------------------------
	----------------------------------------------------------------------------------------------------------------------------------*/
#pragma region Helper_Functions  

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

	FbxAMatrix GetGeometryTransformation(FbxNode* inNode);

	XMMATRIX FBXToXMMatrix(const FbxAMatrix& inMatrix);

	void ProcessSkeletonHierarchyRecursively(FbxNode* inNode, int inDepth, int myIndex, int inParentIndex, TransformNode* curTransform)
	{
		TransformNode* nextParent = nullptr;
		TransformNode* child = new TransformNode();

		if (inNode->GetNodeAttribute() && inNode->GetNodeAttribute()->GetAttributeType() && inNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
		{

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
		}
		for (int i = 0; i < inNode->GetChildCount(); i++)
		{
			ProcessSkeletonHierarchyRecursively(inNode->GetChild(i), inDepth + 1, (int)tomsSkeleton.transforms.size(), myIndex, nextParent);
		}
	}

	void ProcessSkeletonHierarchy(FbxNode* inRootNode)
	{
		TransformNode* root = nullptr;

		for (int childIndex = 0; childIndex < inRootNode->GetChildCount(); ++childIndex)
		{
			FbxNode* currNode = inRootNode->GetChild(childIndex);

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
			currPosition.x = (float)(currMesh->GetControlPointAt(i).mData[0]);
			currPosition.y = (float)(currMesh->GetControlPointAt(i).mData[1]);
			currPosition.z = -(float)(currMesh->GetControlPointAt(i).mData[2]);
			currCtrlPoint->mPosition = currPosition;
			mControlPoints[i] = currCtrlPoint;
		}
	}

	XMFLOAT3 ReadBinormal(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter)
	{
		XMFLOAT3 outBinormal;

		if (inMesh->GetElementBinormalCount() < 1) { return outBinormal; }

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
		return outBinormal;
	}

	XMFLOAT3 ReadTangent(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter)
	{
		XMFLOAT3 outTangent;
		if (inMesh->GetElementTangentCount() < 1) { return outTangent; }

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
		return outTangent;
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
				break;
			}

			case FbxGeometryElement::eIndexToDirect:
			{
				int index = vertexNormal->GetIndexArray().GetAt(inCtrlPointIndex);
				outNormal.x = (float)(vertexNormal->GetDirectArray().GetAt(index).mData[0]);
				outNormal.y = (float)(vertexNormal->GetDirectArray().GetAt(index).mData[1]);
				outNormal.z = (float)(vertexNormal->GetDirectArray().GetAt(index).mData[2]);
				break;
			}

			default:
				break;
			}

		case FbxGeometryElement::eByPolygonVertex:
			switch (vertexNormal->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				outNormal.x = (float)(vertexNormal->GetDirectArray().GetAt(inVertexCounter).mData[0]);
				outNormal.y = (float)(vertexNormal->GetDirectArray().GetAt(inVertexCounter).mData[1]);
				outNormal.z = (float)(vertexNormal->GetDirectArray().GetAt(inVertexCounter).mData[2]);
				break;
			}

			case FbxGeometryElement::eIndexToDirect:
			{
				int index = vertexNormal->GetIndexArray().GetAt(inVertexCounter);
				outNormal.x = (float)(vertexNormal->GetDirectArray().GetAt(index).mData[0]);
				outNormal.y = (float)(vertexNormal->GetDirectArray().GetAt(index).mData[1]);
				outNormal.z = (float)(vertexNormal->GetDirectArray().GetAt(index).mData[2]);
				break;
			}

			default:
				break;
			}
		}
		outNormal.z = -outNormal.z;
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
		FbxQuaternion rotation = inMatrix.GetQ();

		return XMMatrixSet(
			(float)(inMatrix.Get(0, 0)), (float)(inMatrix.Get(0, 1)), -(float)(inMatrix.Get(0, 2)), (float)(inMatrix.Get(0, 3)),
			(float)(inMatrix.Get(1, 0)), (float)(inMatrix.Get(1, 1)), -(float)(inMatrix.Get(1, 2)), (float)(inMatrix.Get(1, 3)),
			-(float)(inMatrix.Get(2, 0)), -(float)(inMatrix.Get(2, 1)), (float)(inMatrix.Get(2, 2)), -(float)(inMatrix.Get(2, 3)),
			(float)(inMatrix.Get(3, 0)), (float)(inMatrix.Get(3, 1)), -(float)(inMatrix.Get(3, 2)), (float)(inMatrix.Get(3, 3)));
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
			FbxSkin* currSkin = (FbxSkin*)(currMesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));

			if (!currSkin) { continue; }

			//get the animation's info
			FbxAnimStack* currAnimStack = mFBXScene->GetSrcObject<FbxAnimStack>(0);
			FbxString animStackName = currAnimStack->GetName();
			FbxTakeInfo* takeInfo = mFBXScene->GetTakeInfo(animStackName);
			FbxTime start = takeInfo->mLocalTimeSpan.GetStart();
			FbxTime end = takeInfo->mLocalTimeSpan.GetStop();

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
					//FbxAMatrix transformMatrix;
					//FbxAMatrix transformLinkMatrix;
					//FbxAMatrix globalBindposeInverseMatrix;

					//currCluster->GetTransformMatrix(transformMatrix);	// The transformation of the mesh at binding time
					//currCluster->GetTransformLinkMatrix(transformLinkMatrix);	// The transformation of the cluster(joint) at binding time from joint space to world space
					//globalBindposeInverseMatrix = transformLinkMatrix.Inverse() * transformMatrix * geometryTransform;

					// Update the information in mSkeleton 
					//tomsSkeleton.transforms[currJointIndex]->world = FBXToXMMatrix(globalBindposeInverseMatrix);

					//because I added a inverse bind pose, I don't need this
					FbxTime currTime;
					currTime.SetFrame(i, FbxTime::eFrames24);
					FbxAMatrix currentTransformOffset = inNode->EvaluateGlobalTransform(currTime) * geometryTransform;
					XMFLOAT4X4 world;
					XMStoreFloat4x4(&world, FBXToXMMatrix(currentTransformOffset.Inverse() * currCluster->GetLink()->EvaluateGlobalTransform(currTime)));
					tempBone.SetWorld(world);

					//set inverse bind pose of bone
					//XMFLOAT4X4 tempBindPoseInverse;
					//XMStoreFloat4x4(&tempBindPoseInverse, FBXToXMMatrix(globalBindposeInverseMatrix));
					//tempBone.SetInverseBindPose(tempBindPoseInverse);

					//set name of bone
					tempBone.SetName(currJointName);

					//push back tempBone into current keyframe
					tomKeyFrame.SetTime((float)currTime.GetSecondDouble());
					tomKeyFrame.InsertBone(tempBone);
				}

				//push back current keyframe into vector of all keyframes
				tomKeyFrames.push_back(tomKeyFrame);
			}

			// Associate each joint with the control points it affects and set the skeleton's inverse bind poses
			unsigned int numOfClusters = currSkin->GetClusterCount();
			tomsSkeleton.inverseBindPoses.resize(numOfClusters);

			for (unsigned int clusterIndex = 0; clusterIndex < numOfClusters; ++clusterIndex)
			{
				FbxCluster* currCluster = currSkin->GetCluster(clusterIndex);
				std::string currJointName = currCluster->GetLink()->GetName();
				unsigned int currJointIndex = FindJointIndexUsingName(currJointName);
				FbxAMatrix transformMatrix;
				FbxAMatrix transformLinkMatrix;
				FbxAMatrix globalBindposeInverseMatrix;

				//get global bind pose inverse
				currCluster->GetTransformMatrix(transformMatrix);	// The transformation of the mesh at binding time
				currCluster->GetTransformLinkMatrix(transformLinkMatrix);	// The transformation of the cluster(joint) at binding time from joint space to world space
				globalBindposeInverseMatrix = transformLinkMatrix.Inverse() * transformMatrix * geometryTransform;

				// set the skeleton's inverse bind pose
				XMFLOAT4X4 tempBindPoseInverse;
				XMStoreFloat4x4(&tempBindPoseInverse, FBXToXMMatrix(globalBindposeInverseMatrix));
				tomsSkeleton.inverseBindPoses[clusterIndex] = tempBindPoseInverse;

				//set up blendweight and blend indices
				unsigned int numOfIndices = currCluster->GetControlPointIndicesCount();
				for (unsigned int i = 0; i < numOfIndices; ++i)
				{
					VertexBlendingInfo currBlendingIndexWeightPair;
					currBlendingIndexWeightPair.mBlendingIndex = currJointIndex;
					currBlendingIndexWeightPair.mBlendingWeight = currCluster->GetControlPointWeights()[i];
					mControlPoints[currCluster->GetControlPointIndices()[i]]->mBlendingInfo.push_back(currBlendingIndexWeightPair);
				}
			}

		}

	}

	void StoreBlendingInfo(Vertex& temp, const std::vector<VertexBlendingInfo>& vertInfos)
	{
		temp.mBlendingIndices = XMINT4(0, 0, 0, 0);
		temp.mBlendingWeight = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

		switch (vertInfos.size())
		{
		default:
			temp.mBlendingWeight = XMFLOAT4(0.25f, 0.25f, 0.25f, 0.25f);
			break;
		case 1:
			temp.mBlendingIndices.x = vertInfos[0].mBlendingIndex;
			temp.mBlendingWeight.x = (float)vertInfos[0].mBlendingWeight;
			break;
		case 2:
			temp.mBlendingIndices.x = vertInfos[0].mBlendingIndex;
			temp.mBlendingIndices.y = vertInfos[1].mBlendingIndex;
			temp.mBlendingWeight.x = (float)vertInfos[0].mBlendingWeight;
			temp.mBlendingWeight.y = (float)vertInfos[1].mBlendingWeight;
			break;
		case 3:
			temp.mBlendingIndices.x = vertInfos[0].mBlendingIndex;
			temp.mBlendingIndices.y = vertInfos[1].mBlendingIndex;
			temp.mBlendingIndices.z = vertInfos[2].mBlendingIndex;
			temp.mBlendingWeight.x = (float)vertInfos[0].mBlendingWeight;
			temp.mBlendingWeight.y = (float)vertInfos[1].mBlendingWeight;
			temp.mBlendingWeight.z = (float)vertInfos[2].mBlendingWeight;
			break;
		case 4:
			temp.mBlendingIndices = XMINT4(vertInfos[0].mBlendingIndex, vertInfos[1].mBlendingIndex, vertInfos[2].mBlendingIndex, vertInfos[3].mBlendingIndex);
			temp.mBlendingWeight = XMFLOAT4((float)vertInfos[0].mBlendingWeight, (float)vertInfos[1].mBlendingWeight, (float)vertInfos[2].mBlendingWeight, (float)vertInfos[3].mBlendingWeight);
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
				temp.mBinormal = ReadBinormal(currMesh, ctrlPointIndex, vertexCounter);
				temp.mTangent = ReadTangent(currMesh, ctrlPointIndex, vertexCounter);

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

	void CleanupFBX()
	{
		mFBXScene->Destroy();
		mFBXManager->Destroy();
		mFBXManager = nullptr;
		mFBXScene = nullptr;

		mVerts.clear();
		tomsSkeleton.names.clear();
		tomsSkeleton.transforms.clear();
		tomKeyFrames.clear();
		transformNodeindex = 0;
		friendlyNodes.clear();
	}

	void ExportBasicMesh(const char * name)
	{
		std::ofstream bout;
		std::string path;
		unsigned int numVerts = 0, numIndices = 0;

		path = "../Resources/";
		path += name;
		path += "/";
		path += name;
		path += ".bmesh";

		bout.open(path, std::ios::binary); //will truncate existing file

		if (bout.is_open())
		{
			//get length of bones
			numVerts = (unsigned int)mBasicVerts.size();
			numIndices = (unsigned int)mIndices.size();

			//write header
			bout.write((const char*)&numVerts, sizeof(unsigned int));
			bout.write((const char*)&numIndices, sizeof(unsigned int));

			//write out Vert data
			bout.write((const char*)mBasicVerts.data(), sizeof(VS_BasicInput) * numVerts);

			//write out indicies
			bout.write((const char*)mIndices.data(), sizeof(unsigned int) * numIndices);
		}

		bout.close();
	}

	void MakeFriendlyNodeRecursive(TransformNode* tNode)
	{
		FriendlyIOTransformNode friendlyNode;

		if (tNode)
		{
			friendlyNode.parentIndex = tNode->parent->index;
			friendlyNode.nameOffset = friendlyNodes[friendlyNode.parentIndex].nameOffset + (unsigned int)tNode->name.size();
			DirectX::XMStoreFloat4x4(&friendlyNode.world, tNode->world);

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
		DirectX::XMStoreFloat4x4(&friendlyNode.world, tNode->world);

		friendlyNodes.push_back(friendlyNode);

		MakeFriendlyNodeRecursive(tNode->child);
	}

	void ExportMesh(const char * name)
	{
		std::ofstream bout;
		std::string path;
		unsigned int numVerts = 0, numIndices = 0;

		path = "../Resources/";
		path += name;
		path += "/";
		path += name;
		path += ".mesh";

		bout.open(path, std::ios::binary); //will truncate existing file

		if (bout.is_open())
		{
			//get length of bones
			numVerts = (unsigned int)mVerts.size();
			numIndices = (unsigned int)mIndices.size();

			//write header
			bout.write((const char*)&numVerts, sizeof(unsigned int));
			bout.write((const char*)&numIndices, sizeof(unsigned int));


			//write out Vert data
			bout.write((const char*)mVerts.data(), sizeof(Vertex) * numVerts);


			//write out indicies
			bout.write((const char*)mIndices.data(), sizeof(unsigned int) * numIndices);
		}

		bout.close();
	}

	void ExportSkeleton(const char * name)
	{
		std::ofstream bout;
		std::string path;
		unsigned int numBones = 0, namesSize = 0;

		path = "../Resources/";
		path += name;
		path += "/";
		path += name;
		path += ".skel";

		bout.open(path, std::ios::binary); //will truncate existing file

		if (bout.is_open())
		{
			//get length of bones
			numBones = (unsigned int)tomsSkeleton.transforms.size();
			namesSize = (unsigned int)tomsSkeleton.names.size();

			//write header
			bout.write((const char*)&numBones, sizeof(unsigned int));
			bout.write((const char*)&namesSize, sizeof(unsigned int));

			MakeFriendlyNode(tomsSkeleton.transforms[0]);

			//write out transform data
			bout.write((const char*)friendlyNodes.data(), sizeof(FriendlyIOTransformNode) * friendlyNodes.size());

			//write out names
			bout.write((const char*)tomsSkeleton.names.data(), namesSize);

			//write out inverse bind poses
			bout.write((const char*)tomsSkeleton.inverseBindPoses.data(), sizeof(DirectX::XMFLOAT4X4) * numBones);
		}

		bout.close();
	}

	void ExportAnimation(const char * name, const char * animationName)
	{
		std::ofstream bout;
		std::string path;

		//now animation file
		path = "../Resources/";
		path += name;
		path += "/";
		path += animationName;
		path += ".anim";

		bout.open(path, std::ios::binary);

		if (bout.is_open())
		{
			//make animation
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
	void ExportToBinary(const char * name, const char* animationName)
	{
		ExportSkeleton(name);

		//load in the animation if there's an animation name
		if (animationName)
		{
			ExportAnimation(name, animationName);
		}

		ExportMesh(name);
	}
#pragma endregion 
	/*----------------------------------------------------------------------------------------------------------------------------------
	----------------------------------------------------------------------------------------------------------------------------------*/

	FBXLOADER_API bool Functions::FBXLoadExportFileBasic(const char * inFilePath, const char * name)
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
		if (!fbxImporter->Initialize(inFilePath, -1, mFBXManager->GetIOSettings())) return false;

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
						vert.position.x = (float)verts[iCtrlPoint].mData[0];
						vert.position.y = (float)verts[iCtrlPoint].mData[1];
						vert.position.z = -(float)verts[iCtrlPoint].mData[2];


						mBasicVerts.push_back(vert);

					}
				}
			}

			mIndices.clear();
			mIndices.resize(mBasicVerts.size());
			ElimanateDuplicates(mBasicVerts, mIndices);

			//swap indices for correct texture
			for (unsigned int i = 0; i < mIndices.size(); i += 3)
			{
				mIndices[i + 1] ^= mIndices[i + 2];
				mIndices[i + 2] ^= mIndices[i + 1];
				mIndices[i + 1] ^= mIndices[i + 2];
			}



			ExportBasicMesh(name);

			return true;
		}
		return false;
	}

	FBXLOADER_API bool Functions::FBXLoadExportAnimation(const char * inFilePath, const char * name, const char * animationName)
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

			ProcessGeometry(mFBXScene->GetRootNode());

			ExportAnimation(name, animationName);

			CleanupFBX();
			return true;
		}
		return false;
	}

	FBXLOADER_API bool Functions::FBXLoadExportSkeleton(const char * inFilePath, const char * name)
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

			ProcessGeometry(mFBXScene->GetRootNode());

			ExportSkeleton(name);

			CleanupFBX();
			return true;
		}
		return false;
	}

	FBXLOADER_API bool Functions::FBXLoadExportMesh(const char * inFilePath, const char * name)
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

			ProcessGeometry(mFBXScene->GetRootNode());

			mIndices.clear();
			mIndices.resize(mVerts.size());
			ElimanateDuplicates(mVerts, mIndices);

			//swap indices for correct texture
			for (unsigned int i = 0; i < mIndices.size(); i += 3)
			{
				mIndices[i + 1] ^= mIndices[i + 2];
				mIndices[i + 2] ^= mIndices[i + 1];
				mIndices[i + 1] ^= mIndices[i + 2];
			}

			ExportMesh(name);

			CleanupFBX();
			return true;
		}
		return false;
	}

	FBXLOADER_API bool Functions::FBXLoadExportFileBind(const char * inFilePath, const char * name, const char* animationName)
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

			ProcessGeometry(mFBXScene->GetRootNode());

			mIndices.clear();
			mIndices.resize(mVerts.size());
			ElimanateDuplicates(mVerts, mIndices);

			//swap indices for correct texture
			for (unsigned int i = 0; i < mIndices.size(); i += 3)
			{
				mIndices[i + 1] ^= mIndices[i + 2];
				mIndices[i + 2] ^= mIndices[i + 1];
				mIndices[i + 1] ^= mIndices[i + 2];
			}

			ExportToBinary(name, animationName);

			CleanupFBX();
			return true;
		}
		return false;

	}

}










