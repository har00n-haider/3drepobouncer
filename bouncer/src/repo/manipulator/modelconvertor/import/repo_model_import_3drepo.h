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
* 3DRepo Geometry convertor(Import)
*/

#pragma once

#include <string>
#include "repo_model_import_abstract.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>

#include "repo_model_import_abstract.h"
#include "../../../core/model/collection/repo_scene.h"
#include "../../../core/model/bson/repo_node_camera.h"
#include "../../../core/model/bson/repo_node_material.h"
#include "../../../core/model/bson/repo_node_mesh.h"
#include "../../../core/model/bson/repo_node_metadata.h"
#include "../../../core/model/bson/repo_node_transformation.h"

namespace repo {
	namespace manipulator {
		namespace modelconvertor {
			const char REPO_IMPORT_TYPE_STRING = 'S';
			const char REPO_IMPORT_TYPE_DOUBLE = 'D';
			const char REPO_IMPORT_TYPE_INT = 'I';
			const char REPO_IMPORT_TYPE_BOOL = 'B';
			const char REPO_IMPORT_TYPE_DATETIME = 'T';

			const std::string REPO_IMPORT_METADATA = "metadata";
			const std::string REPO_IMPORT_GEOMETRY = "geometry";
			const std::string REPO_IMPORT_MATERIAL = "material";

			const std::string REPO_IMPORT_VERTICES = "vertices";
			const std::string REPO_IMPORT_NORMALS = "normals";
			const std::string REPO_IMPORT_INDICES = "indices";

			const std::string REPO_IMPORT_BBOX = "bbox";

			const static int REPO_VERSION_LENGTH = 6;

			class RepoModelImport : public AbstractModelImport
			{
			private:
				const std::string REPO_V1 = "BIM001";
				const std::string REPO_V2 = "BIM002";
				const std::string REPO_V3 = "BIM003";
				const std::set<std::string> supportedFileVersions = 
				{ 
					REPO_V1, 
					REPO_V2,
					REPO_V3
				};

				const int REPO_V1_FILEMETA_BYTE_LEN = 56;
				const int REPO_V2_FILEMETA_BYTE_LEN = REPO_V1_FILEMETA_BYTE_LEN;
				const int REPO_V3_FILEMETA_BYTE_LEN = 72;

				typedef struct
				{
					int64_t jsonSize;			//!< Size of the entire JSON segment
					int64_t dataSize;			//!< Size of the entire binary footer segment
					int64_t sizesStart;			//!< Starting location of the JSON sizes array from the top of file in bytes 
					int64_t sizesSize;			//!< Length of the JSON sizes array in bytes
					int64_t matStart;			//!< Starting location of the JSON materials array from the top of the file in bytes 
					int64_t matSize;			//!< Size of the JSON materials array in bytes
					int64_t numChildren;		//!< Number of children of the root node
					int64_t textureStart = -1;	//!< Starting location of the JSON textures array from the top of the file in bytes 
					int64_t textureSize  = -1;  //!< Size of the JSON textures array in bytes
				} fileMeta;

				struct mesh_data_t 
				{
					std::vector<repo::lib::RepoVector3D64> rawVertices;
					std::vector<repo::lib::RepoVector3D> normals;
					std::vector<repo_face_t> faces;
					std::vector<std::vector<double>> boundingBox;
					repo::lib::RepoUUID parent;
					repo::lib::RepoUUID sharedID;
				};

				repo::core::model::MaterialNode* parseMaterial(const boost::property_tree::ptree &pt);
				repo::core::model::MetadataNode*  createMetadataNode(const boost::property_tree::ptree &metadata, const std::string &parentName, const repo::lib::RepoUUID &parentID);
				mesh_data_t createMeshRecord(const boost::property_tree::ptree &geometry, const std::string &parentName, const repo::lib::RepoUUID &parentID, const repo::lib::RepoMatrix &trans);

				/**
				 * @brief Creates a property tree from the current
				 * position in the fine input stream (fin)
				 * @param number of chars to read
				 * @return boost poperty tree
				*/
				boost::property_tree::ptree getNextJSON(long jsonSize);
				void skipAheadInFile(long amount);

				/**
				 * @brief Creates relevant nodes for given child
				 * of the root node in the BIM file
				 * Directly updates:
				 * trans_matrix_map
				 * node_map
				 * transformations
				 * @param tree 
				*/
				void createObject(const boost::property_tree::ptree& tree);

				// File handling variables
				std::string orgFile;
				std::ifstream *finCompressed;
				boost::iostreams::filtering_streambuf<boost::iostreams::input> *inbuf;
				std::istream *fin;

				// Source file meta data storage
				bool areVertices32Bit = false;
				fileMeta file_meta;
				std::vector<long> sizes; //!< Sizes of the nodes component, used for navigation.
				char *dataBuffer;
				
				// Intermediary variables used to keep track of node hierarchy
				std::vector<repo::core::model::RepoNode *> node_map;			//!< List of all transform nodes in order of decoding
				std::vector<repo::lib::RepoMatrix> trans_matrix_map;			//!< List of all transformation matrices in same order as node_map
				std::vector<repo::core::model::MaterialNode *> matNodeList;		//!< Stores a list of materials
				std::vector<std::vector<repo::lib::RepoUUID>> matParents;		//!< Stores the UUIDs of all parents of a given material node in the same order matNodeList
				std::vector<repo::core::model::MaterialNode *> textureNodeList;	//!< Stores a list of textures
				std::vector<std::vector<repo::lib::RepoUUID>> textureParents;	//!< Stores the UUIDs of all parents of a given texture node the same order as textureNodelList
				std::vector<mesh_data_t> meshEntries;

				// Variables directly used to instantiate the RepoScene
				repo::core::model::RepoNodeSet cameras;
				repo::core::model::RepoNodeSet materials;
				repo::core::model::RepoNodeSet metadata;
				repo::core::model::RepoNodeSet transformations;
				repo::core::model::RepoNodeSet textures;
				std::vector<double> offset;

			public:

				/**
				* Create RepoModelImport with specific settings
				* NOTE: The destructor will destroy the settings object referenced
				* in this object!
				* @param settings
				*/
				RepoModelImport(const ModelImportConfig &settings);

				/**
				* Default Deconstructor
				* NOTE: The destructor will destroy the settings object referenced
				* in this object!
				*/
				virtual ~RepoModelImport();

				/**
				* Generates a repo scene graph
				* an internal representation needs to have
				* been created before this call (e.g. by means of importModel())
				* @return returns a populated RepoScene upon success.
				*/
				virtual repo::core::model::RepoScene* generateRepoScene(uint8_t &errMsg);

				/**
				* Import model from a given file.
				* Loads material nodes.
				* This does not generate the Repo Scene Graph
				* Use generateRepoScene() to generate a Repo Scene Graph.
				* @param path to the file
				* @param error message if failed
				* @return returns true upon success
				*/
				virtual bool importModel(std::string filePath, uint8_t &errMsg);
			};

			//http://stackoverflow.com/questions/23481262/using-boost-property-tree-to-read-int-array
			template <typename T>
			inline std::vector<T> as_vector(const boost::property_tree::ptree &pt, const boost::property_tree::ptree::key_type &key)
			{
				std::vector<T> r;
				for (const auto& item : pt.get_child(key))
					r.push_back(item.second.get_value<T>());
				return r;
			}

			template <typename T>
			inline std::vector<T> as_vector(const boost::property_tree::ptree &pt)
			{
				std::vector<T> r;
				for (const auto& item : pt)
					r.push_back(item.second.get_value<T>());
				return r;
			}
		}
	}
}
