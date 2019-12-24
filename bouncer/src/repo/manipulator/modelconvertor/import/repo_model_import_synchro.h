/**
*  Copyright (C) 2019 3D Repo Ltd
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

#include <string>
#ifdef SYNCHRO_SUPPORT
#include <synchro_reader/spm_reader.h>
#endif
#include <memory>

#include "repo_model_import_abstract.h"
#include "../../../core/model/collection/repo_scene.h"
#include "../../../core/model/bson/repo_node_material.h"
#include "../../../core/model/bson/repo_node_mesh.h"
#include "../../../core/model/bson/repo_node_metadata.h"
#include "../../../core/model/bson/repo_node_transformation.h"

namespace repo{
	namespace manipulator{
		namespace modelconvertor{
			class SynchroModelImport : public AbstractModelImport
			{
			public:
				/**
				* Default Constructor, generate model with default settings
				*/
				SynchroModelImport() {}

				/**
				* Default Deconstructor
				*/
				~SynchroModelImport() {}

				virtual bool requireReorientation() const { return true; }

#ifdef SYNCHRO_SUPPORT
				/**
				* Generates a repo scene graph
				* an internal representation(aiscene) needs to have
				* been created before this call
				* @return returns a populated RepoScene upon success.
				*/
				repo::core::model::RepoScene* generateRepoScene();

				/**
				* Import model from a given file
				* @param path to the file
				* @param error message if failed
				* @return returns true upon success
				*/
				bool importModel(std::string filePath, uint8_t &errMsg);

			private:
				std::pair<repo::core::model::RepoNodeSet, repo::core::model::RepoNodeSet> generateMatNodes(
					std::unordered_map<std::string, repo::lib::RepoUUID> &synchroIDtoRepoID,
					std::unordered_map<repo::lib::RepoUUID, repo::core::model::RepoNode*, repo::lib::RepoUUIDHasher> &repoIDToNode);

				repo::core::model::MetadataNode* createMetaNode(
					const std::unordered_map<std::string, std::string> &metadata,
					const std::string &name,
					const std::vector<repo::lib::RepoUUID> &parents);

				repo::core::model::TransformationNode* createTransNode(
					const repo::lib::RepoMatrix &matrix,
					const std::string &name,
					const std::vector<repo::lib::RepoUUID> &parents = std::vector<repo::lib::RepoUUID>());

				repo::core::model::MeshNode* createMeshNode(
					const repo::core::model::MeshNode &templateMesh,
					const std::vector<double> &transformation,
					const std::vector<double> &offset,
					const repo::lib::RepoUUID &parentID);

				repo::core::model::RepoScene* constructScene(
					std::unordered_map<std::string, std::vector<repo::lib::RepoUUID>> &resourceIDsToSharedIDs);


				uint32_t colourIn32Bit(const std::vector<float> &color) const;

				std::vector<float> colourFrom32Bit(const uint32_t &color) const;


				std::pair<std::string, std::vector<uint8_t>> generateCache(
					const std::unordered_map<repo::lib::RepoUUID, std::pair<float, float>, repo::lib::RepoUUIDHasher> &meshAlphaState,
					const std::unordered_map<repo::lib::RepoUUID, std::pair<uint32_t, std::vector<float>>, repo::lib::RepoUUIDHasher> &meshColourState);

				void addTasks(
					std::unordered_map<std::string, repo::core::model::RepoSequence::Task> &currentTasks,
					std::vector<std::string> &toAdd,
					std::map<std::string, synchro_reader::Task> &tasks
				);

				void removeTasks(
					std::unordered_map<std::string, repo::core::model::RepoSequence::Task> &currentTasks,
					std::vector<std::string> &toRemove,
					std::map<std::string, synchro_reader::Task> &tasks
				);


				void updateFrameState(
					const std::vector<std::shared_ptr<synchro_reader::AnimationTask>> &tasks,
					std::unordered_map<std::string, std::vector<repo::lib::RepoUUID>> &resourceIDsToSharedIDs,
					std::unordered_map<repo::lib::RepoUUID, std::pair<float, float>, repo::lib::RepoUUIDHasher> &meshAlphaState,
					std::unordered_map<repo::lib::RepoUUID, std::pair<uint32_t, std::vector<float>>, repo::lib::RepoUUIDHasher> &meshColourState

				);

				std::unordered_map<std::string, repo::core::model::MeshNode> createMeshTemplateNodes();

				std::shared_ptr<synchro_reader::SPMReader> reader;

				std::string orgFile;
#else
				/**
				* Generates a repo scene graph
				* an internal representation(aiscene) needs to have
				* been created before this call
				* @return returns a populated RepoScene upon success.
				*/
				repo::core::model::RepoScene* generateRepoScene() { return nullptr; }

				/**
				* Import model from a given file
				* @param path to the file
				* @param error message if failed
				* @return returns true upon success
				*/
				bool importModel(std::string filePath, uint8_t &errMsg) {
					errMsg = REPOERR_SYNCHRO_UNAVAILABLE;
					return false;
				}
#endif
			};
		} //namespace SynchroModelImport
	} //namespace manipulator
} //namespace repo
