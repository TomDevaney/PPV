// FBXLoader.cpp : Defines the exported functions for the DLL application.  

#include "FBXLoader.h" 
#include <fbxsdk.h>
#include <unordered_map>
#include <algorithm>
#include <fstream>
#include "Animation.h"
#include "KeyFrame.h"
#include "TransformNode.h"
#include <map>

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
	unsigned int nameOffset;

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
				tomsSkeleton.names.push_back('\0');
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
				tomsSkeleton.names.push_back('\0');
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
			{
				switch (vertexBinormal->GetReferenceMode())
				{
					case FbxGeometryElement::eDirect:
					{
						outBinormal.x = (float)(vertexBinormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[0]) * (float)(vertexBinormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[3]);
						outBinormal.y = (float)(vertexBinormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[1]) * (float)(vertexBinormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[3]);
						outBinormal.z = (float)(vertexBinormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[2]) * (float)(vertexBinormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[3]);
						break;
					}

					case FbxGeometryElement::eIndexToDirect:
					{
						int index = vertexBinormal->GetIndexArray().GetAt(inCtrlPointIndex);
						outBinormal.x = (float)(vertexBinormal->GetDirectArray().GetAt(index).mData[0]) * (float)(vertexBinormal->GetDirectArray().GetAt(index).mData[3]);
						outBinormal.y = (float)(vertexBinormal->GetDirectArray().GetAt(index).mData[1]) * (float)(vertexBinormal->GetDirectArray().GetAt(index).mData[3]);
						outBinormal.z = (float)(vertexBinormal->GetDirectArray().GetAt(index).mData[2]) * (float)(vertexBinormal->GetDirectArray().GetAt(index).mData[3]);
						break;
					}

					default:
						break;
				}
			}
			break;

			case FbxGeometryElement::eByPolygonVertex:
			{
				switch (vertexBinormal->GetReferenceMode())
				{
				case FbxGeometryElement::eDirect:
				{
					outBinormal.x = (float)(vertexBinormal->GetDirectArray().GetAt(inVertexCounter).mData[0]) * (float)(vertexBinormal->GetDirectArray().GetAt(inVertexCounter).mData[3]);
					outBinormal.y = (float)(vertexBinormal->GetDirectArray().GetAt(inVertexCounter).mData[1]) * (float)(vertexBinormal->GetDirectArray().GetAt(inVertexCounter).mData[3]);
					outBinormal.z = (float)(vertexBinormal->GetDirectArray().GetAt(inVertexCounter).mData[2]) * (float)(vertexBinormal->GetDirectArray().GetAt(inVertexCounter).mData[3]);
					break;
				}

				case FbxGeometryElement::eIndexToDirect:
				{
					int index = vertexBinormal->GetIndexArray().GetAt(inVertexCounter);
					outBinormal.x = (float)(vertexBinormal->GetDirectArray().GetAt(index).mData[0]) * (float)(vertexBinormal->GetDirectArray().GetAt(index).mData[3]);
					outBinormal.y = (float)(vertexBinormal->GetDirectArray().GetAt(index).mData[1]) * (float)(vertexBinormal->GetDirectArray().GetAt(index).mData[3]);
					outBinormal.z = (float)(vertexBinormal->GetDirectArray().GetAt(index).mData[2]) * (float)(vertexBinormal->GetDirectArray().GetAt(index).mData[3]);
					break;
				}

				default:
					break;
				}
			}
			break;
		}

		outBinormal.z = -outBinormal.z;

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
			{
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
			}
			break;

			case FbxGeometryElement::eByPolygonVertex:
			{
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
			break;
		}

		outTangent.z = -outTangent.z;

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
			{
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
			}
			break;

			case FbxGeometryElement::eByPolygonVertex:
			{
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
			break;
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

		FbxLayerElement::EReferenceMode ref = vertexNormal->GetReferenceMode();
		FbxLayerElement::EMappingMode mode = vertexNormal->GetMappingMode();

		switch (vertexNormal->GetMappingMode())
		{
			case FbxGeometryElement::eByControlPoint:
			{
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
					{
				

					}
					break;
				}
			}
			break;

			case FbxGeometryElement::eByPolygonVertex:
			{
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
					{
					}
					break;
				}
			}
			break;
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

		unsigned int indexOffset = 0;
		std::map<unsigned int, std::string> names;
		FbxSkin* currSkin2 = (FbxSkin*)(currMesh->GetDeformer(0, FbxDeformer::eSkin));

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
	
			FbxTime::EMode mode = mFBXScene->GetGlobalSettings().GetTimeMode();

			unsigned int numOfClusters2 = currSkin->GetClusterCount();
			for (unsigned int i = 0; i < numOfClusters2; ++i)
			{
				FbxCluster* currCluster2 = currSkin2->GetCluster(i);
				std::string currJointName = currCluster2->GetLink()->GetName();
				names[i] = currJointName;
			}

			FbxTime tweenTime;

			tweenTime.SetFrame(1, mode);

			//for each keyframe, loop through all of the bones and get their world matrix at that keyframe
			for (FbxLongLong i = start.GetFrameCount(mode); i <= end.GetFrameCount(mode); ++i)
			{
				//for each cluster
				//unsigned int numOfClusters = currSkin->GetClusterCount();
				KeyFrame tomKeyFrame;
				FbxTime currTime;
				currTime.SetFrame(i, mode);
				indexOffset = 0;

				for (unsigned int clusterIndex = 0; clusterIndex < tomsSkeleton.transforms.size(); ++clusterIndex)
				{
					Bone tempBone;
					FbxCluster* currCluster;
					std::string currJointName;

					if (clusterIndex - indexOffset < numOfClusters2)
					{
						currCluster = currSkin->GetCluster(clusterIndex - indexOffset);
					}
					else //this should only happen if the last bone in the skeleton isn't in any of the clusters
					{
						currCluster = currSkin->GetCluster(clusterIndex - indexOffset - 1);
					}

					currJointName = currCluster->GetLink()->GetName();

					FbxAMatrix currentTransformOffset = inNode->EvaluateGlobalTransform(currTime) * geometryTransform;
					XMFLOAT4X4 world;

					//Set world of bone at the current keyframe
					if (currJointName == tomsSkeleton.transforms[clusterIndex]->name)
					{
						XMStoreFloat4x4(&world, FBXToXMMatrix(currentTransformOffset.Inverse() * currCluster->GetLink()->EvaluateGlobalTransform(currTime)));
						tempBone.SetWorld(world);

						//set name of bone
						tempBone.SetName(currJointName);
					}
					else //if it's not in the skeleton, add world of the previous cluster
					{
						currCluster = currSkin->GetCluster(clusterIndex - indexOffset - 1); //use the previous cluster as the world matrix for this bone
						currJointName = currCluster->GetLink()->GetName();

						XMStoreFloat4x4(&world, FBXToXMMatrix(currentTransformOffset.Inverse() * currCluster->GetLink()->EvaluateGlobalTransform(currTime)));
						tempBone.SetWorld(world);

						++indexOffset;
						
						//set name of bone
						tempBone.SetName(tomsSkeleton.transforms[clusterIndex]->name);
					}

					//push back tempBone into current keyframe
					tomKeyFrame.InsertBone(tempBone);
				}

				//push back current keyframe into vector of all keyframes
				tomKeyFrame.SetTime((float)currTime.GetSecondDouble());
				tomKeyFrame.SetTweenTime((float)tweenTime.GetSecondDouble());
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

		//make up for missing bones

		indexOffset = 0;

		for (int i = 0; i < tomsSkeleton.transforms.size(); ++i)
		{
			if (tomsSkeleton.transforms[i]->name != names[i - indexOffset]) //if the keyframe's bones doesn't have a skeleton bone
			{
				tomsSkeleton.inverseBindPoses.insert(tomsSkeleton.inverseBindPoses.begin() + i, tomsSkeleton.inverseBindPoses[i - 1]);
				++indexOffset; // because this means I skipped a bone in the skeleton, I need to use this to look back
				//tomsSkeleton.inverseBindPoses.insert(tomsSkeleton.inverseBindPoses.begin() + i, XMFLOAT4X4(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
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
	XMFLOAT3 cross_product(XMFLOAT3 lhs, XMFLOAT3 rhs)
	{
		XMFLOAT3 Result;
		Result.x = (lhs.y * rhs.z) - (lhs.z * rhs.y);
		Result.y = (lhs.z * rhs.x) - (lhs.x * rhs.z);
		Result.z = (lhs.x * rhs.y) - (lhs.y * rhs.x);
		return Result;

	}
	float dot_product(XMFLOAT3 lhs, XMFLOAT3 rhs)
	{
		return (lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z);
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
				XMFLOAT3 tmpTan =  ReadTangent(currMesh, ctrlPointIndex, vertexCounter);
				temp.mBinormal = cross_product(temp.mNormal, tmpTan);// ReadBinormal(currMesh, ctrlPointIndex, vertexCounter);
		
				temp.mTangent.x = tmpTan.x; temp.mTangent.y = tmpTan.y; temp.mTangent.z = tmpTan.z;
				temp.mTangent.w = (dot_product(cross_product(temp.mNormal, tmpTan), temp.mBinormal) < 0.0F) ? -1.0F : 1.0F;

				//temp.mTangent.w = (temp.mPosition.x <0.0f) ? -1.0F : 1.0F;
				if (temp.mTangent.w != -1.0f)
				{
					int x = 0;
					x = x * 2;
				}
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

				if (temp.mNormal.z < 0.0f)
				{
					int z = 0;

					z++;
				}

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
		//tomsSkeleton.names.clear();
		tomsSkeleton.transforms.clear();
		tomKeyFrames.clear();
		transformNodeindex = 0;
		friendlyNodes.clear();
		nameOffset = 0;
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
			//friendlyNode.nameOffset = friendlyNodes[friendlyNode.parentIndex].nameOffset + (unsigned int)tNode->name.size();
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
		//friendlyNode.nameOffset = 0;
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

			//set name offsets in friendly node
			friendlyNodes[0].nameOffset = 0;

			for (int i = 1; i < tomsSkeleton.transforms.size(); ++i)
			{
				nameOffset += (unsigned int)tomsSkeleton.transforms[i - 1]->name.size() + 1; //plus 1 for null terminator
				friendlyNodes[i].nameOffset = nameOffset;
			}

			//write out transform data
			bout.write((const char*)friendlyNodes.data(), sizeof(FriendlyIOTransformNode) * friendlyNodes.size());

			//write out names
			//for (int i = 0; i < tomsSkeleton.transforms.size(); ++i)
			//{
			//	nameSize = tomsSkeleton.transforms[i]->name.size();

			//	bout.write((const char*)&nameSize, sizeof(unsigned int));
			//	bout.write((const char*)tomsSkeleton.transforms[i]->name.data(), nameSize);

			//	bout.write((const char*)&friendlyNodes[i].world, sizeof(DirectX::XMFLOAT4X4));
			//	bout.write((const char*)&friendlyNodes[i].local, sizeof(DirectX::XMFLOAT4X4));
			//	bout.write((const char*)&friendlyNodes[i].parentIndex, sizeof(int));
			//	bout.write((const char*)&friendlyNodes[i].childIndex, sizeof(int));
			//	bout.write((const char*)&friendlyNodes[i].siblingIndex, sizeof(int));
			//	bout.write((const char*)&friendlyNodes[i].bDirty, sizeof(bool));
			//}



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
				float tweenTime = tomKeyFrames[i].GetTweenTime();

				bout.write((const char*)tomKeyFrames[i].GetBones().data(), sizeof(Bone) * numOfBones);
				bout.write((const char*)&keyFrameTime, sizeof(float));
				bout.write((const char*)&tweenTime, sizeof(float));
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










