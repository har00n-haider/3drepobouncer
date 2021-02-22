/**
*  Copyright (C) 2018 3D Repo Ltd
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

#pragma once

#include "../../../../core/model/bson/repo_bson_factory.h"
#include "../../../../lib/datastructure/repo_structs.h"
#include "helper_functions.h"

#include <fstream>
#include <vector>
#include <string>
#include <boost/uuid/uuid_generators.hpp>

namespace repo {
	namespace manipulator {
		namespace modelconvertor {
			namespace odaHelper {

				struct camera_t
				{
					float aspectRatio;
					float farClipPlane;
					float nearClipPlane;
					float FOV;
					repo::lib::RepoVector3D eye;
					repo::lib::RepoVector3D pos;
					repo::lib::RepoVector3D up;
					std::string name;
				};


				struct mesh_data_t {
					std::vector<repo::lib::RepoVector3D64> rawVertices;
					std::vector<repo::lib::RepoVector3D64> rawNormals;
					std::vector<repo_face_t> faces;
					std::vector<std::vector<float>> boundingBox;
					std::multimap<repo::lib::RepoVector3D64, 
						std::pair<int, repo::lib::RepoVector3D64>, 
						RepoVector3D64SortComparator> vToVIndex;
					std::vector<repo::lib::RepoVector2D> uvCoords;
					std::string name;
					std::string layerName;
					std::string groupName;
					uint32_t matIdx;
				};

				class GeometryCollector
				{
				public:
					GeometryCollector();
					~GeometryCollector();
					
					/**
					* Check whether collector has missing textures.
					* @return returns true if at least one texture is missing
					*/
					bool hasMissingTextures();

					/**
					* Get all the material and texture nodes collected.
					* @return returns a repoNodeSet containing material nodes
					*/
					void getMaterialAndTextureNodes(repo::core::model::RepoNodeSet& materials, repo::core::model::RepoNodeSet& textures);


					/**
					* Get all the transformation nodes collected.
					* This is based on the layer information
					* @return returns a repoNodeSet containing transformation nodes
					*/
					repo::core::model::RepoNodeSet getTransformationNodes() {
						return transNodes;
					}

					/**
					* Get all the metadata nodes collected.
					* This is based on the layer information
					* @return returns a repoNodeSet containing metadata nodes
					*/
					repo::core::model::RepoNodeSet getMetadataNodes() {
						return metaNodes;
					}

					/**
					* Get all mesh nodes collected.
					* @return returns a vector of mesh nodes
					*/
					repo::core::model::RepoNodeSet getMeshNodes(const repo::core::model::TransformationNode& root);

					/**
					* Gt the model offset applied on the meshes collected
					* this is typically the minimum bounding box.
					* @return returns a vector of double (3 values) for the model offset.
					*/
					std::vector<double> getModelOffset() const {
						std::vector<double> res;
						for (int i = 0; i < minMeshBox.size(); ++i) {
							res.push_back(origin.size() > i ? minMeshBox[i] - origin[i] : minMeshBox[i]);
						}

						return res;
					}

					/**
					* returns a boolean indicating if we already have metedata for this given ID
					* @return returns true if there is a metadata entry
					*/
					bool hasMeta(const std::string &id) {
						return idToMeta.find(id) != idToMeta.end();
					}

					/**
					* Indicates a start of a mesh
					*/
					void startMeshEntry();

					/**
					* Indicates end of a mesh
					*/
					void stopMeshEntry();

					void setLayer(const std::string id, const std::string &name) {
						nextLayer = id;
						if (layerIDToName.find(id) == layerIDToName.end())
							layerIDToName[id] = name;
					}

					/**
					* Set the name for the next mesh
					* @param name name of the next mesh
					*/
					void setNextMeshName(const std::string &name) {
						nextMeshName = name;
					}

					/**
					* Set next group name
					* @param groupName group name of the next mesh
					* @return returns true if there's already a metadata entry for this grouping
					*/
					void setMeshGroup(const std::string &groupName) {
						nextGroupName = groupName;
					}

					/**
					* Add a face to the current mesh, setting the normal for all the vertices
					* @param vertices a vector of vertices that makes up this face
					*/
					void addFace(
						const std::vector<repo::lib::RepoVector3D64> &vertices,
						const repo::lib::RepoVector3D64& normal,
						const std::vector<repo::lib::RepoVector2D>& uvCoords = std::vector<repo::lib::RepoVector2D>()
					);

					/**
					* Add a face to the current mesh. This has no normals or uvs (such as would be the case with polylines)
					* @param vertices a vector of vertices that makes up this face
					*/
					void addFace(
						const std::vector<repo::lib::RepoVector3D64>& vertices
					);

					/**
					* Change current material to the one provided
					* @param material material contents.
					*/
					void setCurrentMaterial(const repo_material_t &material, bool missingTexture = false);

					void setOrigin(const double &x, const double &y, const double &z) {
						origin = { x, y, z };
					}

					/**
					* Change current meta node to the one provided
					* @param meta node
					*/
					void setCurrentMeta(const std::map<std::string, std::string>& meta);

					/**
					* Get all meta nodes collected.
					* @return returns a vector of meta nodes
					*/
					repo::core::model::RepoNodeSet getMetaNodes();

					/**
					* Set transformation matrix for rootNode
					* @param transformation matrix for rootNode
					*/
					void setRootMatrix(repo::lib::RepoMatrix matrix);

					/**
					* Set additional camera for scene
					* @param camera node
					*/
					void addCameraNode(repo::manipulator::modelconvertor::odaHelper::camera_t node);

					/**
					* Returns true if cameras are available
					*/
					bool hasCameraNodes();

					/**
					* Get cameras for scene
					* @return cameras
					*/
					repo::core::model::RepoNodeSet getCameraNodes(repo::lib::RepoUUID parentID);
					
					/**
					* Create transformation root node
					* @return root transformation node
					*/
					repo::core::model::TransformationNode createRootNode();

					/**
					* Set metadata of a group
					* @param groupName groupName
					* @param metaEntry Metadata entry for groupName
					*/
					void setMetadata(const std::string &groupName,
						const std::unordered_map<std::string, std::string> &metaEntry)
					{
						if(metaEntry.size() > 0)
							idToMeta[groupName] = metaEntry;
					}


				private:

					std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<int, std::unordered_map<std::string, mesh_data_t>>>> meshData;
					std::unordered_map<std::string, std::unordered_map<std::string, std::string> > idToMeta;
					std::unordered_map<std::string, std::string> layerIDToName;
					std::string nextMeshName, nextLayer, nextGroupName;
					std::unordered_map< uint32_t, std::pair<repo::core::model::MaterialNode, repo::core::model::TextureNode> > idxToMat;
					std::unordered_map<uint32_t, std::vector<repo::lib::RepoUUID> > matToMeshes;
					repo::core::model::RepoNodeSet transNodes, metaNodes;
					uint32_t currMat;
					std::vector<double> minMeshBox, origin;

					mesh_data_t *currentEntry = nullptr;
					bool missingTextures = false;
					repo::lib::RepoMatrix rootMatrix;
					std::vector<repo::manipulator::modelconvertor::odaHelper::camera_t> cameras;

					repo::core::model::TransformationNode* createTransNode(
						const std::string &name,
						const std::string &id,
						const repo::lib::RepoUUID &parentId
					);

					repo::core::model::TextureNode createTextureNode(const std::string& texturePath);
					repo::core::model::MetadataNode*  createMetaNode(
						const std::string &name,
						const repo::lib::RepoUUID &parentId,
						const  std::unordered_map<std::string, std::string> &metaValues
					);

					void startOrContinueMeshByFormat(bool hasUvs, bool hasNormals, int faceSize);

					mesh_data_t createMeshEntry();

					boost::uuids::random_generator uuidGenerator; // Common generator because they are apparently expensive to seed (https://stackoverflow.com/questions/3247861)
				};
			}
		}
	}
}

