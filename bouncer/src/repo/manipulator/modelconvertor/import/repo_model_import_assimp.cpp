/**
*  Copyright (C) 2015 3D Repo Ltd
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU Affero General Public License as
*  published by the Free Software Foundation, either version 3 of the
*  License, or (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU Affero General Public License for more details.
*
*  You should have received a copy of the GNU Affero General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
* Allows Import functionality into/output Repo world using ASSIMP
*/

#include "repo_model_import_assimp.h"

#include <algorithm>
#include <fstream>
#include <regex>
#include <boost/filesystem.hpp>

#include <assimp/importerdesc.h>

#include "../../../core/model/bson/repo_bson_builder.h"
#include "../../../core/model/bson/repo_bson_factory.h"
#include "../../../lib/repo_utils.h"
#include "../../../error_codes.h"
#include "./repo_model_import_config_default_values.h"

using namespace repo::manipulator::modelconvertor;

AssimpModelImport::AssimpModelImport(const ModelImportConfig &settings) :
	AbstractModelImport(settings)
{
}

AssimpModelImport::~AssimpModelImport()
{
	if (assimpScene)
		importer.FreeScene();
}
bool AssimpModelImport::isSupportedExts(const std::string &testExt)
{
	std::list<std::string> res;
	Assimp::Importer importer;
	aiString ext;
	importer.GetExtensionList(ext);

	// all file extensions in convenient all package

	std::string str(ext.C_Str());
	std::string lowerExt(testExt);
	repo::lib::toLower(lowerExt);

	return str.find(lowerExt) != std::string::npos;
}
std::string AssimpModelImport::getSupportedFormats()
{
	Assimp::Importer importer;
	aiString ext;
	importer.GetExtensionList(ext);

	// all file extensions in convenient all package

	std::string str(ext.C_Str());

	std::replace(str.begin(), str.end(), ';', ' ');

	std::string all = "All (" + str + ")";

	// individual extensions by name
	std::string individual = "";
	for (size_t i = 0; i < importer.GetImporterCount(); ++i)
	{
		const aiImporterDesc *desc = importer.GetImporterInfo(i);
		std::string str(desc->mFileExtensions);

		//put *. in front of the 2nd+ file extensions (as some might have multiple extensions)
		std::string extension = std::regex_replace(str, std::regex(" "), " *.");

		//remove "Importer" from the name
		std::string formatName = std::regex_replace(std::string(desc->mName), std::regex(" Importer"), "");
		individual += ";;" + formatName + " (*." + extension + ")";
	}
	return all + individual;
}

bool AssimpModelImport::requireReorientation() const {
	return requiresOrientation;
}

uint32_t AssimpModelImport::composeAssimpPostProcessingFlags(
	uint32_t flag)
{
	if (repoDefaultCalculateTangentSpace)
		flag |= aiProcess_CalcTangentSpace;

	if (repoDefaultConvertToUVCoordinates)
		flag |= aiProcess_GenUVCoords;

	if (repoDefaultDegeneratesToPointsLines)
		flag |= aiProcess_FindDegenerates;

	if (repoDefaultDebone)
		flag |= aiProcess_Debone;

	// Debone threshold
	// Debone only if all

	if (repoDefaultFindInstances)
		flag |= aiProcess_FindInstances;

	if (repoDefaultFindInvalidData)
		flag |= aiProcess_FindInvalidData;

	// Animation accuracy

	if (repoDefaultFixInfacingNormals)
		flag |= aiProcess_FixInfacingNormals;

	if (repoDefaultFlipUVCoordinates)
		flag |= aiProcess_FlipUVs;

	if (repoDefaultFlipWindingOrder)
		flag |= aiProcess_FlipWindingOrder;

	if (repoDefaultGenerateNormals && repoDefaultGenerateNormalsFlat)
		flag |= aiProcess_GenNormals;

	if (repoDefaultGenerateNormals && repoDefaultGenerateNormalsSmooth)
		flag |= aiProcess_GenSmoothNormals;

	// Crease angle

	if (repoDefaultImproveCacheLocality)
		flag |= aiProcess_ImproveCacheLocality;

	// Vertex cache size

	if (repoDefaultJoinIdenticalVertices)
		flag |= aiProcess_JoinIdenticalVertices;

	if (repoDefaultLimitBoneWeights)
		flag |= aiProcess_LimitBoneWeights;

	// Max bone weights

	if (repoDefaultMakeLeftHanded)
		flag |= aiProcess_MakeLeftHanded;

	if (repoDefaultOptimizeMeshes)
		flag |= aiProcess_OptimizeMeshes;

	if (repoDefaultPreTransformUVCoordinates)
		flag |= aiProcess_TransformUVCoords;

	if (repoDefaultPreTransformVertices)
	{
		repoWarning << "PretransformVertices flag is set. If you want to generate multipart stash disable this flag as it has been migrated to RepoBouncer.";
		flag |= aiProcess_PreTransformVertices;
	}

	// Normalize

	if (repoDefaultRemoveComponents)
		flag |= aiProcess_RemoveComponent;

	// !individual components!

	if (repoDefaultRemoveRedundantMaterials)
		flag |= aiProcess_RemoveRedundantMaterials;

	// Skip materials

	if (repoDefaultRemoveRedundantNodes)
		flag |= aiProcess_OptimizeGraph;

	// Skip nodes

	if (repoDefaultSortAndRemove)
		flag |= aiProcess_SortByPType;

	// Remove types

	if (repoDefaultSplitByBoneCount)
		flag |= aiProcess_SplitByBoneCount;

	// Max bones

	if (repoDefaultSplitLargeMeshes)
		flag |= aiProcess_SplitLargeMeshes;

	// Vertex limit
	// Triangle limit

	if (repoDefaultTriangulate)
		flag |= aiProcess_Triangulate;

	if (repoDefaultValidateDataStructures)
		flag |= aiProcess_ValidateDataStructure;

	return flag;
}

repo::core::model::CameraNode* AssimpModelImport::createCameraRepoNode(
	const aiCamera *assimpCamera,
	const std::vector<double> &worldOffset)
{
	std::string cameraName(assimpCamera->mName.data);

	repo::core::model::CameraNode * cameraNode;
	std::vector<double> offset;
	if (worldOffset.size())
	{
		offset = worldOffset;
	}
	else
	{
		offset = { 0, 0, 0 };
	}
	if (assimpCamera)
	{
		cameraNode = new repo::core::model::CameraNode(repo::core::model::RepoBSONFactory::makeCameraNode(
			assimpCamera->mAspect,
			assimpCamera->mClipPlaneFar,
			assimpCamera->mClipPlaneNear,
			assimpCamera->mHorizontalFOV,
			{ (float)(assimpCamera->mLookAt.x - offset[0]), (float)(assimpCamera->mLookAt.y - offset[1]), (float)(assimpCamera->mLookAt.z - offset[2]) },
			{ (float)(assimpCamera->mPosition.x), (float)(assimpCamera->mPosition.y - offset[1]), (float)(assimpCamera->mPosition.z - offset[2]) },
			{ (float)(assimpCamera->mUp.x - offset[0]), (float)(assimpCamera->mUp.y - offset[1]), (float)(assimpCamera->mUp.z - offset[2]) },
			cameraName
		));
	}

	return cameraNode;
}

float AssimpModelImport::normaliseShininess(const float &rawValue) const {
	std::string ext = getFileExtension(orgFile);
	float value;
	if (ext == ".FBX") {
		value = rawValue / 20.;
	}
	else if (ext == ".OBJ") {
		value = rawValue / 200.;
	}
	else if (ext == ".DAE") {
		value = rawValue > 128 ? 1 : rawValue / 128;
	}
	else {
		value = rawValue / 1000;
	}

	return value;
}

repo::core::model::MaterialNode* AssimpModelImport::createMaterialRepoNode(
	const aiMaterial *material,
	const std::string &name,
	const std::unordered_map<std::string, repo::core::model::RepoNode *> &nameToTexture)
{
	repo::core::model::MaterialNode *materialNode;

	if (material) {
		repo_material_t repo_material;

		aiColor3D tempColor;
		auto tempFloat = tempColor.b;

		//--------------------------------------------------------------------------
		// Ambient
		if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_AMBIENT, tempColor))
		{
			repo_material.ambient.push_back(tempColor.r);
			repo_material.ambient.push_back(tempColor.g);
			repo_material.ambient.push_back(tempColor.b);
		}
		//--------------------------------------------------------------------------
		// Diffuse
		if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_DIFFUSE, tempColor))
		{
			repo_material.diffuse.push_back(tempColor.r);
			repo_material.diffuse.push_back(tempColor.g);
			repo_material.diffuse.push_back(tempColor.b);
		}

		//--------------------------------------------------------------------------
		// Specular
		if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_SPECULAR, tempColor))
		{
			repo_material.specular.push_back(tempColor.r);
			repo_material.specular.push_back(tempColor.g);
			repo_material.specular.push_back(tempColor.b);
		}

		//--------------------------------------------------------------------------
		// Emissive
		if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_EMISSIVE, tempColor))
		{
			repo_material.emissive.push_back(tempColor.r);
			repo_material.emissive.push_back(tempColor.g);
			repo_material.emissive.push_back(tempColor.b);
		}

		//--------------------------------------------------------------------------
		// Wireframe
		int wireframe;
		material->Get(AI_MATKEY_ENABLE_WIREFRAME, wireframe);
		repo_material.isWireframe = (0 != wireframe);

		//--------------------------------------------------------------------------
		// Two-sided
		int twoSided;
		material->Get(AI_MATKEY_TWOSIDED, twoSided);
		repo_material.isTwoSided = (0 != twoSided);

		//--------------------------------------------------------------------------
		// Opacity
		if (AI_SUCCESS == material->Get(AI_MATKEY_OPACITY, tempFloat))
		{
			//If opacity is 0, we assume it's bonkers.
			repo_material.opacity = tempFloat == 0 ? 1 : tempFloat;
		}
		else
			repo_material.opacity = 1;

		//--------------------------------------------------------------------------
		// Shininess
		if (AI_SUCCESS == material->Get(AI_MATKEY_SHININESS, tempFloat))
			repo_material.shininess = normaliseShininess(tempFloat);
		else
			repo_material.shininess = std::numeric_limits<float>::quiet_NaN();
		//--------------------------------------------------------------------------
		// Shininess strength
		if (AI_SUCCESS == material->Get(AI_MATKEY_SHININESS_STRENGTH, tempFloat))
			repo_material.shininessStrength = tempFloat >= 0 ? tempFloat : 0;
		else
			repo_material.shininessStrength = std::numeric_limits<float>::quiet_NaN();

		materialNode = new repo::core::model::MaterialNode(
			repo::core::model::RepoBSONFactory::makeMaterialNode(repo_material, name));

		//--------------------------------------------------------------------------
		// Texture (one diffuse for the moment)
		// Textures are uniquely referenced by their name
		aiString texPath; // contains a filename of a texture
		/*repoTrace << " #None Texture:" << material->GetTextureCount(aiTextureType_NONE);
		repoTrace << " #Diffuse Texture:" << material->GetTextureCount(aiTextureType_DIFFUSE);
		repoTrace << " #Spec Texture:" << material->GetTextureCount(aiTextureType_SPECULAR);
		repoTrace << " #Ambient Texture:" << material->GetTextureCount(aiTextureType_AMBIENT);
		repoTrace << " #Emissive Texture:" << material->GetTextureCount(aiTextureType_EMISSIVE);
		repoTrace << " #Height Texture:" << material->GetTextureCount(aiTextureType_HEIGHT);
		repoTrace << " #Normal Texture:" << material->GetTextureCount(aiTextureType_NORMALS);
		repoTrace << " #Shiniess Texture:" << material->GetTextureCount(aiTextureType_SHININESS);
		repoTrace << " #Opac Texture:" << material->GetTextureCount(aiTextureType_OPACITY);
		repoTrace << " #Disp Texture:" << material->GetTextureCount(aiTextureType_DISPLACEMENT);
		repoTrace << " #LightM Texture:" << material->GetTextureCount(aiTextureType_LIGHTMAP);
		repoTrace << " #Reflection Texture:" << material->GetTextureCount(aiTextureType_REFLECTION);
		repoTrace << " #Unknown Texture:" << material->GetTextureCount(aiTextureType_UNKNOWN);
		*/

		if (AI_SUCCESS == material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath))
		{
			std::unordered_map<std::string, repo::core::model::RepoNode *>::const_iterator it = nameToTexture.find(texPath.data);

			if (nameToTexture.end() != it)
			{
				repo::core::model::RepoNode * nodeAddr = it->second;

				repo::core::model::RepoNode tmp = nodeAddr->cloneAndAddParent(materialNode->getSharedID());
				nodeAddr->swap(tmp);
			}
			else
			{
				repoError << "Could not find related texture mapping";
			}
		}
	}

	return materialNode;
}

repo::core::model::MeshNode AssimpModelImport::createMeshRepoNode(
	const aiMesh *assimpMesh,
	const std::vector<repo::core::model::RepoNode *> &materials,
	std::unordered_map < repo::core::model::RepoNode*, std::vector<repo::lib::RepoUUID>> &matMap,
	const bool hasTexture,
	const std::vector<double> &offset)
{
	repo::core::model::MeshNode meshNode;

	//Avoid using assimp objects everywhere -> converting assimp objects into repo structs
	std::vector<repo::lib::RepoVector3D> vertices;
	std::vector<repo_face_t> faces;
	std::vector<repo::lib::RepoVector3D> normals;
	std::vector<std::vector<repo::lib::RepoVector2D>> uvChannels;
	std::vector<repo_color4d_t> colors;
	std::vector<std::vector<float>>   outline;

	/*
	 *--------------------- Vertices (always present) -----------------------------
	 */
	aiVector3D offsetVec = offset.size() ? aiVector3D(offset[0], offset[1], offset[2]) : aiVector3D(0, 0, 0);
	aiVector3D firstV = assimpMesh->mVertices[0];
	firstV -= offsetVec;
	repo::lib::RepoVector3D minVertex = { (float)firstV.x, (float)firstV.y, (float)firstV.z };
	repo::lib::RepoVector3D maxVertex = minVertex;

	for (uint32_t i = 0; i < assimpMesh->mNumVertices; i++)
	{
		auto aiVertex = assimpMesh->mVertices[i];
		aiVertex -= offsetVec;
		vertices.push_back({ (float)aiVertex.x, (float)aiVertex.y, (float)aiVertex.z });

		minVertex.x = minVertex.x < aiVertex.x ? minVertex.x : aiVertex.x;
		minVertex.y = minVertex.y < aiVertex.y ? minVertex.y : aiVertex.y;
		minVertex.z = minVertex.z < aiVertex.z ? minVertex.z : aiVertex.z;

		maxVertex.x = maxVertex.x > aiVertex.x ? maxVertex.x : aiVertex.x;
		maxVertex.y = maxVertex.y > aiVertex.y ? maxVertex.y : aiVertex.y;
		maxVertex.z = maxVertex.z > aiVertex.z ? maxVertex.z : aiVertex.z;
	}

	/*
	*-----------------------------------------------------------------------------
	*/

	/*
	*------------------------------- Faces ----------------------------------------
	*/
	if (assimpMesh->HasFaces())
	{
		for (uint32_t i = 0; i < assimpMesh->mNumFaces; i++)
		{
			faces.push_back({ std::vector<uint32_t>(assimpMesh->mFaces[i].mIndices,
				assimpMesh->mFaces[i].mIndices + assimpMesh->mFaces[i].mNumIndices) });
		}
	}
	/*
	*-----------------------------------------------------------------------------
	*/

	/*
	*------------------------------- Normals --------------------------------------
	*/
	if (assimpMesh->HasNormals())
	{
		for (uint32_t i = 0; i < assimpMesh->mNumVertices; i++)
		{
			normals.push_back({ (float)assimpMesh->mNormals[i].x, (float)assimpMesh->mNormals[i].y, (float)assimpMesh->mNormals[i].z });
		}
	}
	/*
	*-----------------------------------------------------------------------------
	*/

	/*
	*--------------------------------- Bones ---------------------------------------
	*/
	/*
	TODO:
	if (mesh->HasBones())
	{
	}

	if (mesh->HasPositions())
	{
	}

	if (mesh->HasTangentsAndBitangents())
	{
	}*/
	/*
	*-----------------------------------------------------------------------------
	*/

	/*
	*------------------------------ UV Channels ----------------------------------
	*/
	// Copies only the very first UV channel over
	// TODO: make sure enough memory can be allocated
	// TODO: add support for all UV channels.
	if (assimpMesh->HasTextureCoords(0))
	{
		std::vector<repo::lib::RepoVector2D> channelVector;
		for (uint32_t i = 0; i < assimpMesh->mNumVertices; i++)
		{
			channelVector.push_back({ (float)assimpMesh->mTextureCoords[0][i].x, (float)assimpMesh->mTextureCoords[0][i].y });
		}
		uvChannels.push_back(channelVector);
	}
	else if (hasTexture)
	{
		//Has texture but no UV coordinates, attempt to fabricate some
		std::vector<repo::lib::RepoVector2D> channelVector;

		repo::lib::RepoVector3D bboxSize = { fabsf(maxVertex.x - minVertex.x), fabsf(maxVertex.y - minVertex.y), fabsf(maxVertex.z - minVertex.z) };

		for (const auto & v : vertices)
		{
			repo::lib::RepoVector3D dVector = { fabsf(v.x - minVertex.x), fabsf(v.y - minVertex.y), fabsf(v.z - minVertex.z) };
			channelVector.push_back({ dVector.x / bboxSize.x, dVector.y / bboxSize.y });
		}
		uvChannels.push_back(channelVector);
	}

	// Consider only first color set
	if (assimpMesh->HasVertexColors(0))
	{
		for (uint32_t i = 0; i < assimpMesh->mNumVertices; i++)
		{
			colors.push_back({
				(float)assimpMesh->mColors[0][i].r,
				(float)assimpMesh->mColors[0][i].g,
				(float)assimpMesh->mColors[0][i].b,
				(float)assimpMesh->mColors[0][i].a });
		}
	}
	/*
	*-----------------------------------------------------------------------------
	*/

	/*
	*----------------------------- SHA-256 hash ----------------------------------
	*/
	//FIXME:

	//vertexHash = hash(*vertices, boundingBox);

	/*
	*-----------------------------------------------------------------------------
	*/

	std::vector<std::vector<float>>  boundingBox;
	std::vector<float> minBound, maxBound;
	minBound.push_back(minVertex.x);
	minBound.push_back(minVertex.y);
	minBound.push_back(minVertex.z);

	maxBound.push_back(maxVertex.x);
	maxBound.push_back(maxVertex.y);
	maxBound.push_back(maxVertex.z);

	boundingBox.push_back(minBound);
	boundingBox.push_back(maxBound);

	outline.push_back({ minVertex.x, minVertex.y });
	outline.push_back({ maxVertex.x, minVertex.y });
	outline.push_back({ maxVertex.x, maxVertex.y });
	outline.push_back({ minVertex.x, maxVertex.y });

	meshNode = repo::core::model::MeshNode(repo::core::model::RepoBSONFactory::makeMeshNode(
		vertices, faces, normals, boundingBox, uvChannels, colors, outline));

	///*
	//*------------------------------ setParents ----------------------------------
	//*/

	if (assimpMesh->mMaterialIndex < materials.size())
	{
		repo::core::model::RepoNode *materialNode = materials[assimpMesh->mMaterialIndex];
		matMap[materialNode].push_back(meshNode.getSharedID());
	}

	///*
	//*-----------------------------------------------------------------------------
	//*/

	return meshNode;
}

repo::core::model::MetadataNode* AssimpModelImport::createMetadataRepoNode(
	const aiMetadata             *assimpMeta,
	const std::string            &metadataName,
	const std::vector<repo::lib::RepoUUID> &parents)
{
	repo::core::model::MetadataNode *metaNode;
	std::string val;
	if (assimpMeta)
	{
		//build the metadata as a bson
		repo::core::model::RepoBSONBuilder builder;

		for (uint32_t i = 0; i < assimpMeta->mNumProperties; i++)
		{
			std::string key(assimpMeta->mKeys[i].C_Str());
			aiMetadataEntry &currentValue = assimpMeta->mValues[i];

			if (key == "IfcGloballyUniqueId")
				repoError << "TODO: fix IfcGloballyUniqueId in RepoMetadata" << std::endl;

			switch (currentValue.mType)
			{
			case AI_BOOL:
				builder.append(key, *(static_cast<bool *>(currentValue.mData)));
				break;

			case AI_INT32:
				builder.append(key, *(static_cast<int *>(currentValue.mData)));
				break;

			case AI_UINT64:
				//mongo doesn't support 64bit unsigned. storing it as a signed number
				builder.append(key, (long long)(*(static_cast<uint64_t *>(currentValue.mData))));
				break;

			case AI_FLOAT:
				builder.append(key, *(static_cast<float *>(currentValue.mData)));
				break;

			case AI_AISTRING:
				val = (static_cast<aiString *>(currentValue.mData))->C_Str();

				if (val.compare(key))
					builder.append(key, val);

				break;
			case AI_AIVECTOR3D:
			{
				aiVector3D *vector = (static_cast<aiVector3D *>(currentValue.mData));

				repo::lib::RepoVector3D repoVector = { (float)vector->x, (float)vector->y, (float)vector->z };

				builder.append(key, repoVector);
			}
			break;
			case FORCE_32BIT:
				// Gracefully (but silently) handle the bogus enum used by
				// assimp to ensure the enum is 32-bits.
				break;
			}
		}

		repo::core::model::RepoBSON metaBSON = builder.obj();

		metaNode = new repo::core::model::MetadataNode(
			repo::core::model::RepoBSONFactory::makeMetaDataNode(metaBSON, "", metadataName, parents));
	}//if(assimpMeta)

	return metaNode;
}

repo::core::model::RepoNodeSet AssimpModelImport::createTransformationNodesRecursive(
	const aiNode                                                     *assimpNode,
	const std::unordered_map<std::string, repo::core::model::RepoNode *> &cameras,
	const std::vector<repo::core::model::RepoNode>           &meshes,
	const std::unordered_map<repo::lib::RepoUUID, repo::core::model::RepoNode *, repo::lib::RepoUUIDHasher>    &meshToMat,
	std::unordered_map<repo::core::model::RepoNode *, std::vector<repo::lib::RepoUUID>> &matParents,
	repo::core::model::RepoNodeSet			                 &newMeshes,
	repo::core::model::RepoNodeSet						     &metadata,
	uint32_t                                               &count,
	const std::vector<double>                                &worldOffset,
	const std::vector<repo::lib::RepoUUID>						             &parent

)
{
	repo::core::model::RepoNodeSet transNodes;

	if (assimpNode) {
		std::string transName(assimpNode->mName.data);
		if (count % 1000 == 0)
			repoInfo << "Constructing transformation #" << count;

		//create a 4 by 4 vector
		std::vector < std::vector<float> > transMat;

		for (int i = 0; i < 4; i++) {
			std::vector<float> rows;
			for (int j = 0; j < 4; j++) {
				rows.push_back(assimpNode->mTransformation[i][j]);
			}
			transMat.push_back(rows);
		}

		//We need to update the translation vector with the worldOffset
		if (worldOffset.size())
		{
			for (int i = 0; i < 3; ++i)
			{
				double extraOffset = worldOffset[0] * transMat[i][0] + transMat[i][1] * worldOffset[1]
					+ worldOffset[2] * transMat[i][2] - worldOffset[i];
				transMat[i][3] += extraOffset;
			}
		}

		repo::core::model::TransformationNode * transNode =
			new repo::core::model::TransformationNode(
				repo::core::model::RepoBSONFactory::makeTransformationNode(repo::lib::RepoMatrix(transMat), transName, parent));

		repo::lib::RepoUUID sharedId = transNode->getSharedID();
		std::vector<repo::lib::RepoUUID> myShareID;
		myShareID.push_back(sharedId);

		//--------------------------------------------------------------------------
		// Register meshes as children of this transformation if any
		for (unsigned int i = 0; i < assimpNode->mNumMeshes; ++i)
		{
			unsigned int meshIndex = assimpNode->mMeshes[i];
			if (meshIndex < meshes.size())
			{
				repo::core::model::RepoNode mesh = meshes[meshIndex];

				if (!mesh.isEmpty())
				{
					newMeshes.insert(duplicateMesh(sharedId, mesh, meshToMat, matParents));
				}
			}
		}

		//--------------------------------------------------------------------------
		// Register cameras as children of this transformation (by name) if any
		std::unordered_map<std::string, repo::core::model::RepoNode *>::const_iterator it =
			cameras.find(assimpNode->mName.data);
		if (cameras.end() != it)
		{
			repo::core::model::RepoNode * camera = it->second;
			if (camera)
			{
				repo::core::model::RepoNode tmp = camera->cloneAndAddParent(sharedId);
				camera->swap(tmp);
			}
		}

		//--------------------------------------------------------------------------
		// Collect metadata and add as a child
		if (keepMetadata && assimpNode->mMetaData)
		{
			std::string metadataName = assimpNode->mName.data;
			if (metadataName == "<transformation>")
				metadataName = "<metadata>";

			repo::core::model::MetadataNode *metachild = createMetadataRepoNode(assimpNode->mMetaData, metadataName, myShareID);
			metadata.insert(metachild);
		}

		transNodes.insert(transNode);

		//--------------------------------------------------------------------------
		// Register child transformations as children if any
		for (unsigned int i = 0; i < assimpNode->mNumChildren; ++i)
		{
			repo::core::model::RepoNodeSet childMetadata;
			repo::core::model::RepoNodeSet childSet = createTransformationNodesRecursive(assimpNode->mChildren[i],
				cameras, meshes, meshToMat, matParents, newMeshes, childMetadata, ++count, worldOffset, myShareID);

			transNodes.insert(childSet.begin(), childSet.end());
			metadata.insert(childMetadata.begin(), childMetadata.end());
		}
	} //if assimpNode

	return transNodes;
}

repo::core::model::RepoScene* AssimpModelImport::convertAiSceneToRepoScene()
{
	repo::core::model::RepoScene *scenePtr = nullptr;

	if (assimpScene)
	{
		//Turn everything into repoNodes and construct a scene
		repo::core::model::RepoNodeSet cameras; //!< Cameras
		repo::core::model::RepoNodeSet meshes; //!< Meshes
		repo::core::model::RepoNodeSet materials; //!< Materials
		repo::core::model::RepoNodeSet metadata; //!< Metadata
		repo::core::model::RepoNodeSet transformations; //!< Transformations
		repo::core::model::RepoNodeSet textures;

		std::vector<repo::core::model::RepoNode *> originalOrderMaterial; //vector that keeps track original order for assimp indices
		std::unordered_map<repo::core::model::RepoNode *, std::vector<repo::lib::RepoUUID>> matParents;//Tracks material parents
		std::unordered_map<repo::lib::RepoUUID, repo::core::model::RepoNode*, repo::lib::RepoUUIDHasher> meshToMat;
		std::vector<repo::core::model::RepoNode> originalOrderMesh; //vector that keeps track original order for assimp indices
		std::unordered_map<std::string, repo::core::model::RepoNode *> camerasMap;
		std::unordered_map<std::string, repo::core::model::RepoNode *> nameToTexture;

		std::vector<std::vector<double>> sceneBbox = getSceneBoundingBox();
		//-------------------------------------------------------------------------
		// Textures

		repoInfo << "Constructing Texture Nodes...";
		bool missingTextures = false;

		for (uint32_t m = 0; m < assimpScene->mNumMaterials; ++m)
		{
			int texIndex = 0;
			aiReturn texFound = AI_SUCCESS;
			const aiMaterial *material = assimpScene->mMaterials[m];

			uint32_t nTex = material->GetTextureCount(aiTextureType_DIFFUSE);
			for (uint32_t iTex = 0; iTex < nTex; ++iTex)
			{
				aiString path;	// filename
				if (AI_SUCCESS == material->GetTexture(aiTextureType_DIFFUSE, iTex, &path))
				{
					std::string texName(path.data);
					repoTrace << "texture name: " << texName;

					if (!texName.empty())
					{
						repo::core::model::RepoNode *textureNode = nullptr;

						const aiTexture* texture = nullptr;
						if (texture = assimpScene->GetEmbeddedTexture(texName.c_str()))
						{
							repoTrace << "Embedded texture name: " << texName;
							//---------------------------------------------------------
							// Embedded texture
							char *memblock = (char*)malloc(texture->mWidth);
							memcpy(memblock, texture->pcData, texture->mWidth);

							auto size = texture->mWidth * (texture->mHeight == 0 ? 1 : texture->mHeight);
							textureNode = new repo::core::model::TextureNode(repo::core::model::RepoBSONFactory::makeTextureNode(
								texName,
								(char*)texture->pcData,
								size,
								texture->mWidth,
								texture->mHeight));

							free(memblock);
						}
						else
						{
							repoTrace << "External texture name: " << texName;
							//External texture
							std::ifstream::pos_type size;
							std::string dirPath = getDirPath(orgFile);
							boost::filesystem::path filePath = boost::filesystem::absolute(texName, dirPath);
							std::ifstream file(filePath.string(), std::ios::in | std::ios::binary | std::ios::ate);
							char *memblock = nullptr;
							if (!file.is_open())
							{
								repoError << "Could not open texture: " << filePath << std::endl;
								missingTextures = true;
							}
							else
							{
								size = file.tellg();
								memblock = new char[size];
								file.seekg(0, std::ios::beg);
								file.read(memblock, size);
								file.close();
							}

							textureNode = new repo::core::model::TextureNode(repo::core::model::RepoBSONFactory::makeTextureNode(
								texName,
								memblock,
								size,
								size,
								0));

							if (memblock)delete[] memblock;
						}

						if (textureNode)
						{
							textures.insert(textureNode);
							nameToTexture[texName] = textureNode;
							repoTrace << "Added texture :" << texName;
						}
					}
					else
					{
						repoWarning << "Texture name is empty!";
					}
				}
				else
				{
					repoWarning << "Unable to get texture from material.";
				}
			}
		}
		repoInfo << "Constructing Material Nodes...";
		/*
		* ------------- Material Nodes -----------------
		*/
		// Warning: Default material might not be attached to anything,
		// hence it would not be returned by a call to getNodes().
		if (assimpScene->HasMaterials())
		{
			for (uint32_t i = 0; i < assimpScene->mNumMaterials; ++i)
			{
				if (i % 100 == 0 || i == assimpScene->mNumMaterials - 1)
				{
					repoInfo << "Constructing " << i << " of " << assimpScene->mNumMaterials;
				}
				aiString name;
				assimpScene->mMaterials[i]->Get(AI_MATKEY_NAME, name);

				repo::core::model::RepoNode* material = createMaterialRepoNode(
					assimpScene->mMaterials[i],
					name.data, nameToTexture);

				if (!material)
					repoError << "Unable to construct material node in Assimp Model Convertor!";
				else
				{
					materials.insert(material);
				}

				originalOrderMaterial.push_back(material);
				std::pair<repo::core::model::RepoNode *, std::vector<repo::lib::RepoUUID>> a;
				a.first = material;
				a.second = std::vector<repo::lib::RepoUUID>();
				matParents.insert(a);
			}
		}

		/*
		* ---------------------------------------------
		*/

		repoInfo << "Constructing Mesh Nodes...";
		if (sceneBbox.size())
			repoInfo << "Scene offset : {" << sceneBbox[0][0] << "," << sceneBbox[0][1] << "," << sceneBbox[0][2] << "}";
		else
		{
			repoError << "Could not calculate scene offset, num.Meshes = " << assimpScene->mNumMeshes;
			sceneBbox.push_back({ 0, 0, 0 });
			sceneBbox.push_back({ 0, 0, 0 });
		}
		/*
		* --------------- Mesh Nodes ------------------
		*/
		if (assimpScene->HasMeshes())
		{
			for (unsigned int i = 0; i < assimpScene->mNumMeshes; ++i)
			{
				if (i % 500 == 0 || i == assimpScene->mNumMeshes - 1)
				{
					repoInfo << "Constructing " << i << " of " << assimpScene->mNumMeshes;
				}

				int numTextures = assimpScene->mMaterials[assimpScene->mMeshes[i]->mMaterialIndex]->GetTextureCount(aiTextureType_DIFFUSE);
				repo::core::model::RepoNode mesh = createMeshRepoNode(
					assimpScene->mMeshes[i],
					originalOrderMaterial,
					matParents, numTextures > 0,
					sceneBbox.size() ? sceneBbox[0] : std::vector<double>());

				if (mesh.isEmpty())
					repoError << "Unable to construct mesh node in Assimp Model Convertor!";
				else
				{
					originalOrderMesh.push_back(mesh);
				}
			}

			//Update material parents
			for (const auto &matPair : matParents)
			{
				/*if (matPair.second.size() > 0)
				{
				repo::core::model::RepoNode updatedMat = matPair.first->cloneAndAddParent(matPair.second);
				matPair.first->swap(updatedMat);
				}*/

				for (const auto meshId : matPair.second)
					meshToMat[meshId] = matPair.first;
			}
			matParents.clear();
		}

		/*
		* ---------------------------------------------
		*/

		repoInfo << "Constructing Camera Nodes...";
		/*
		* ------------- Camera Nodes ------------------
		*/

		if (assimpScene->HasCameras())
		{
			for (unsigned int i = 0; i < assimpScene->mNumCameras; ++i)
			{
				if (i % 100 == 0 || i == assimpScene->mNumCameras - 1)
				{
					repoInfo << "Constructing " << i << " of " << assimpScene->mNumCameras;
				}
				std::string cameraName(assimpScene->mCameras[i]->mName.data);
				repo::core::model::RepoNode* camera = createCameraRepoNode(assimpScene->mCameras[i], sceneBbox.size() ? sceneBbox[0] : std::vector<double>());
				if (!camera)
					repoError << "Unable to construct mesh node in Assimp Model Convertor!";
				else
				{
					cameras.insert(camera);
				}

				camerasMap.insert(std::make_pair(cameraName, camera));
			}
		}
		/*
		* ---------------------------------------------
		*/
		//--------------------------------------------------------------------------
		// TODO: Animations
		//if (assimpScene->HasAnimations())
		//{
		//}

		//--------------------------------------------------------------------------
		// TODO: Lights
		//if (assimpScene->HasLights())
		//{
		//}

		// TODO: Bones

		repoInfo << "Constructing Transformation Nodes...";
		/*
		* ----------- Transformation Nodes ------------
		*/
		// Recursively converts aiNode and all of its children to a hierarchy
		// of RepoNodeTransformations. Call with root node of aiScene.
		// RootNode will be the first entry in transformations vector.

		uint32_t count = 0;
		transformations = createTransformationNodesRecursive(assimpScene->mRootNode,
			camerasMap, originalOrderMesh, meshToMat, matParents, meshes, metadata, count, sceneBbox[0]);

		repoInfo << "Node Construction completed. (#transformations: " << transformations.size() << ", #Metadata" << metadata.size() << ")";

		//Update material parents
		for (const auto &matPair : matParents)
		{
			if (matPair.second.size() > 0)
			{
				repo::core::model::RepoNode updatedMat = matPair.first->cloneAndAddParent(matPair.second);
				matPair.first->swap(updatedMat);
			}
		}

		/*
		* ---------------------------------------------
		*/

		std::vector<std::string> fileVect;
		if (!orgFile.empty())
			fileVect.push_back(orgFile);
		scenePtr = new repo::core::model::RepoScene(fileVect, cameras, meshes, materials, metadata, textures, transformations);
		if (missingTextures)
		{
			scenePtr->setMissingTexture();
		}
		scenePtr->setWorldOffset(sceneBbox[0]);
	}
	else
	{
		repoError << "Failed to load scene from file (aiScene is null)";
	}//if(assimpScene)

	return scenePtr;
}

repo::core::model::RepoNode* AssimpModelImport::duplicateMesh(
	repo::lib::RepoUUID                    &newParent,
	repo::core::model::RepoNode &mesh,
	const std::unordered_map<repo::lib::RepoUUID, repo::core::model::RepoNode *, repo::lib::RepoUUIDHasher>    &meshToMat,
	std::unordered_map<repo::core::model::RepoNode *, std::vector<repo::lib::RepoUUID>> &matParents)
{
	auto newMesh = new repo::core::model::MeshNode(mesh.cloneAndAddParent(newParent, true, true, true));
	auto it = meshToMat.find(mesh.getSharedID());
	if (it != meshToMat.end() && it->second)
	{
		if (matParents.find(it->second) == matParents.end())
			matParents[it->second] = std::vector<repo::lib::RepoUUID>();

		matParents[it->second].push_back(newMesh->getSharedID());
	}
	return newMesh;
}

repo::core::model::RepoScene * AssimpModelImport::generateRepoScene(uint8_t &errMsg)
{
	repo::core::model::RepoScene *scene;

	//Make sure we are using 64bit (issue 4 branch) of assimp
	aiVector3D test;
	if (sizeof(test.x) != sizeof(double))
	{
		repoWarning << "Bouncer library is compiled against a 32bit assimp library. Results may be sub-optimal.";
	}

	//This will generate the non optimised scene
	repoTrace << "Converting AiScene to repoScene";
	importer.ApplyPostProcessing(composeAssimpPostProcessingFlags());
	scene = convertAiSceneToRepoScene();

	return scene;
}

std::vector<std::vector<double>> AssimpModelImport::getSceneBoundingBox() const
{
	std::vector<std::vector<double>> bbox;
	if (assimpScene)
	{
		//default constructor is identity
		const aiMatrix4x4 identity;

		getSceneBoundingBoxInternal(assimpScene->mRootNode, identity, bbox);
	}

	return bbox;
}

void AssimpModelImport::getSceneBoundingBoxInternal(
	const aiNode                     *node,
	const aiMatrix4x4                &mat,
	std::vector<std::vector<double>> &bbox) const
{
	const aiMatrix4x4 transformation = mat * node->mTransformation;

	if (node->mNumMeshes)
	{
		for (size_t meshIdx = 0; meshIdx < node->mNumMeshes; ++meshIdx)
		{
			std::vector<std::vector<double>> meshBbox = getAiMeshBoundingBox(assimpScene->mMeshes[meshIdx]);
			if (meshBbox.size() == 2)
			{
				aiVector3D vectorMin(meshBbox[0][0], meshBbox[0][1], meshBbox[0][2]);
				aiVector3D scaledBoundaryMin = transformation * vectorMin;

				aiVector3D vectorMax(meshBbox[1][0], meshBbox[1][1], meshBbox[1][2]);
				aiVector3D scaledBoundaryMax = transformation * vectorMax;

				if (bbox.size())
				{
					if (scaledBoundaryMin.x < bbox[0][0])
						bbox[0][0] = scaledBoundaryMin.x;
					if (scaledBoundaryMin.y < bbox[0][1])
						bbox[0][1] = scaledBoundaryMin.y;
					if (scaledBoundaryMin.z < bbox[0][2])
						bbox[0][2] = scaledBoundaryMin.z;

					if (scaledBoundaryMax.x > bbox[1][0])
						bbox[1][0] = scaledBoundaryMax.x;
					if (scaledBoundaryMax.y > bbox[1][1])
						bbox[1][1] = scaledBoundaryMax.y;
					if (scaledBoundaryMax.z > bbox[1][2])
						bbox[1][2] = scaledBoundaryMax.z;
				}
				else
				{
					bbox.push_back({ scaledBoundaryMin.x, scaledBoundaryMin.y, scaledBoundaryMin.z });
					bbox.push_back({ scaledBoundaryMax.x, scaledBoundaryMax.y, scaledBoundaryMax.z });
				}
			}
		}
	}

	for (size_t childIdx = 0; childIdx < node->mNumChildren; ++childIdx)
	{
		getSceneBoundingBoxInternal(node->mChildren[childIdx], transformation, bbox);
	}
}

std::vector<std::vector<double>> AssimpModelImport::getAiMeshBoundingBox(
	const aiMesh *mesh) const
{
	std::vector<std::vector<double>> bbox;
	if (mesh->mNumVertices)
	{
		bbox.push_back({ mesh->mVertices[0].x, mesh->mVertices[0].y, mesh->mVertices[0].z });
		bbox.push_back({ mesh->mVertices[0].x, mesh->mVertices[0].y, mesh->mVertices[0].z });

		for (size_t vIdx = 1; vIdx < mesh->mNumVertices; ++vIdx)
		{
			auto currentV = mesh->mVertices[vIdx];

			if (bbox[0][0] < currentV.x)
				bbox[0][0] = currentV.x;
			if (bbox[0][1] < currentV.y)
				bbox[0][1] = currentV.y;
			if (bbox[0][2] < currentV.z)
				bbox[0][2] = currentV.z;

			if (bbox[1][0] > currentV.x)
				bbox[1][0] = currentV.x;
			if (bbox[1][1] > currentV.y)
				bbox[1][1] = currentV.y;
			if (bbox[1][2] > currentV.z)
				bbox[1][2] = currentV.z;
		}
	}
	else
	{
		repoWarning << "Mesh with no vertices found!";
	}

	return bbox;
}

std::string AssimpModelImport::getFileExtension(const std::string &filePath) const {
	boost::filesystem::path filePathP(filePath);
	std::string fileExt = filePathP.extension().string();

	std::transform(fileExt.begin(), fileExt.end(), fileExt.begin(), ::toupper);

	return fileExt;
}

bool AssimpModelImport::importModel(std::string filePath, uint8_t &err)
{
	bool success = true;
	orgFile = filePath;
	setAssimpProperties();

	//-------------------------------------------------------------------------
	// Import model

	std::string fileName = getFileName(filePath);

	repoInfo << "IMPORT [" << fileName << "]";

	//check if a file exist first
	std::ifstream fs(filePath);
	if (!fs.is_open() || !fs.good())
	{
		repoDebug << "Failed to find file";
		err = REPOERR_MODEL_FILE_READ;

		return false;
	}

	keepMetadata = getFileExtension(filePath) == ".IFC";
	importer.SetPropertyInteger(AI_CONFIG_GLOB_MEASURE_TIME, 1);
	importer.SetPropertyBool(AI_CONFIG_IMPORT_IFC_SKIP_SPACE_REPRESENTATIONS, repoDefaultIfcSkipSpaceRepresentation);
	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_READ_TEXTURES, true);
	assimpScene = importer.ReadFile(filePath, 0);

	if (!assimpScene) {
		std::string errorString = importer.GetErrorString();
		repoError << " Failed to convert file to aiScene : " << errorString;
		if (errorString.find("format version") != std::string::npos) {
			if (errorString.find("FBX") != std::string::npos) {
				err = REPOERR_UNSUPPORTED_FBX_VERSION;
			}
			else {
				err = REPOERR_UNSUPPORTED_VERSION;
			}
		}
		else
			err = REPOERR_FILE_ASSIMP_GEN;
		success = false;
	}
	else
	{
		// try to set the orientation from metadata
		bool rootOrientatonSet = SetRootOrientationFromMetadata();
		std::string ext = repo::lib::getExtension(orgFile);
		repo::lib::toLower(ext);
		bool isFbxFile = ext == ".fbx";
		requiresOrientation = !rootOrientatonSet && isFbxFile;

		//-------------------------------------------------------------------------
		// Polygon count
		uint64_t polyCount = 0;
		for (unsigned int i = 0; i < assimpScene->mNumMeshes; ++i)
			polyCount += assimpScene->mMeshes[i]->mNumFaces;

		repoInfo << "=== IMPORTING MODEL WITH ASSIMP MODEL CONVERTOR ===";
		repoInfo << "Loaded " << fileName << " with " << polyCount << " polygons in " << assimpScene->mNumMeshes << " " << ((assimpScene->mNumMeshes == 1) ? "mesh" : "meshes");
	}

	return success;
}

void AssimpModelImport::setAssimpProperties() {
	if (repoDefaultCalculateTangentSpace)
		importer.SetPropertyFloat(AI_CONFIG_PP_CT_MAX_SMOOTHING_ANGLE, repoDefaultCalculateTangentSpaceMaxSmoothingAngle);

	if (repoDefaultDebone)
	{
		importer.SetPropertyFloat(AI_CONFIG_PP_DB_THRESHOLD, repoDefaultDeboneThreshold);
		importer.SetPropertyBool(AI_CONFIG_PP_DB_ALL_OR_NONE, repoDefaultDeboneOnlyIfAll);
	}

	if (repoDefaultFindInvalidData)
		importer.SetPropertyFloat(AI_CONFIG_PP_FID_ANIM_ACCURACY, repoDefaultFindInvalidDataAnimationAccuracy);

	if (repoDefaultGenerateNormals && repoDefaultGenerateNormalsSmooth)
		importer.SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, repoDefaultGenerateNormalsSmoothCreaseAngle);

	if (repoDefaultImproveCacheLocality)
		importer.SetPropertyInteger(AI_CONFIG_PP_ICL_PTCACHE_SIZE, repoDefaultImproveCacheLocalityVertexCacheSize);

	if (repoDefaultLimitBoneWeights)
		importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, repoDefaultLimitBoneWeightsMaxWeight);

	if (repoDefaultPreTransformVertices)
		importer.SetPropertyBool(AI_CONFIG_PP_PTV_NORMALIZE, repoDefaultPreTransformVerticesNormalize);

	if (repoDefaultRemoveComponents) {
		int32_t removeComponents = 0;
		removeComponents |= repoDefaultRemoveComponentsAnimations ? aiComponent_ANIMATIONS : 0;
		removeComponents |= repoDefaultRemoveComponentsBiTangents ? aiComponent_TANGENTS_AND_BITANGENTS : 0;
		removeComponents |= repoDefaultRemoveComponentsBoneWeights ? aiComponent_BONEWEIGHTS : 0;
		removeComponents |= repoDefaultRemoveComponentsCameras ? aiComponent_CAMERAS : 0;
		removeComponents |= repoDefaultRemoveComponentsColors ? aiComponent_COLORS : 0;
		removeComponents |= repoDefaultRemoveComponentsLights ? aiComponent_LIGHTS : 0;
		removeComponents |= repoDefaultRemoveComponentsMaterials ? aiComponent_MATERIALS : 0;
		removeComponents |= repoDefaultRemoveComponentsMeshes ? aiComponent_MESHES : 0;
		removeComponents |= repoDefaultRemoveComponentsNormals ? aiComponent_NORMALS : 0;
		removeComponents |= repoDefaultRemoveComponentsTextureCoordinates ? aiComponent_TEXCOORDS : 0;
		removeComponents |= repoDefaultRemoveComponentsTextures ? aiComponent_TEXTURES : 0;

		importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, removeComponents);
	}

	if (repoDefaultRemoveRedundantMaterials)
		importer.SetPropertyString(AI_CONFIG_PP_RRM_EXCLUDE_LIST, repoDefaultRemoveRedundantMaterialsSkip);

	if (repoDefaultRemoveRedundantNodes)
		importer.SetPropertyString(AI_CONFIG_PP_OG_EXCLUDE_LIST, repoDefaultRemoveRedundantNodesSkip);

	if (repoDefaultSortAndRemove)
	{
		int32_t removePrimitives = 0;
		removePrimitives |= repoDefaultSortAndRemovePoints ? aiPrimitiveType_POINT : 0;
		removePrimitives |= repoDefaultSortAndRemoveLines ? aiPrimitiveType_LINE : 0;
		removePrimitives |= repoDefaultSortAndRemoveTriangles ? aiPrimitiveType_TRIANGLE : 0;
		removePrimitives |= repoDefaultSortAndRemovePolygons ? aiPrimitiveType_POLYGON : 0;
		importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, removePrimitives);
	}
	if (repoDefaultSplitLargeMeshes)
	{
		importer.SetPropertyInteger(AI_CONFIG_PP_SLM_TRIANGLE_LIMIT, repoDefaultSplitLargeMeshesTriangleLimit);
		importer.SetPropertyInteger(AI_CONFIG_PP_SLM_VERTEX_LIMIT, repoDefaultSplitLargeMeshesVertexLimit);
	}
	if (repoDefaultSplitByBoneCount)
		importer.SetPropertyInteger(AI_CONFIG_PP_SBBC_MAX_BONES, repoDefaultSplitByBoneCountMaxBones);
}

bool AssimpModelImport::SetRootOrientationFromMetadata()
{
	if (!assimpScene || !assimpScene->mMetaData) return false;

	// assumed axis, if no metadata is found 
	// 0 - x 
	// 1 - y
	// 2 - z
	int32_t upAxis = 1;
	int32_t upAxisSign = 1;
	int32_t frontAxis = 2;
	int32_t frontAxisSign = 1;
	int32_t coordAxis = 0;
	int32_t coordAxisSign = 1;

	// values will only be populated if key exists
	bool reqMetadataExists = true;
	reqMetadataExists &= assimpScene->mMetaData->Get<int32_t>("UpAxis", upAxis);
	reqMetadataExists &= assimpScene->mMetaData->Get<int32_t>("UpAxisSign", upAxisSign);
	reqMetadataExists &= assimpScene->mMetaData->Get<int32_t>("FrontAxis", frontAxis);
	reqMetadataExists &= assimpScene->mMetaData->Get<int32_t>("FrontAxisSign", frontAxisSign);
	reqMetadataExists &= assimpScene->mMetaData->Get<int32_t>("CoordAxis", coordAxis);
	reqMetadataExists &= assimpScene->mMetaData->Get<int32_t>("CoordAxisSign", coordAxisSign);
	if (!reqMetadataExists) return false;

	// create the transformation
	aiVector3D uV;
	aiVector3D fV;
	aiVector3D rV;
	uV[upAxis] = upAxisSign;
	fV[frontAxis] = frontAxisSign;
	rV[coordAxis] = coordAxisSign;
	aiMatrix4x4 orientationCorrection = aiMatrix4x4(
		rV.x, rV.y, rV.z, 0.0f,
		uV.x, uV.y, uV.z, 0.0f,
		fV.x, fV.y, fV.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	// apply to scene
	assimpScene->mRootNode->mTransformation *= orientationCorrection;
	repoInfo << "Set the root orientation from metadata";
	return true;
}