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

#include "repo_bson_factory.h"

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include "repo_bson_builder.h"
#include "../../../lib/repo_log.h"

using namespace repo::core::model;

RepoBSON RepoBSONFactory::appendDefaults(
	const std::string &type,
	const unsigned int api,
	const repo::lib::RepoUUID &sharedId,
	const std::string &name,
	const std::vector<repo::lib::RepoUUID> &parents,
	const repo::lib::RepoUUID &uniqueID)
{
	RepoBSONBuilder builder;
	uint64_t bytesize = 0;

	//--------------------------------------------------------------------------
	// ID field (UUID)
	builder.append(REPO_NODE_LABEL_ID, uniqueID);

	//--------------------------------------------------------------------------
	// Shared ID (UUID)
	builder.append(REPO_NODE_LABEL_SHARED_ID, sharedId);

	bytesize += 2 * sizeof(repo::lib::RepoUUID);

	//--------------------------------------------------------------------------
	// Type
	if (!type.empty())
	{
		builder.append(REPO_NODE_LABEL_TYPE, type);
	}

	//--------------------------------------------------------------------------
	// Parents
	if (parents.size() > 0)
	{
		builder.appendArray(REPO_NODE_LABEL_PARENTS, parents);
	}

	//--------------------------------------------------------------------------
	// Name
	if (!name.empty())
	{
		builder.append(REPO_NODE_LABEL_NAME, name);
	}

	return builder.obj();
}

CameraNode RepoBSONFactory::makeCameraNode(
	const float         &aspectRatio,
	const float         &farClippingPlane,
	const float         &nearClippingPlane,
	const float         &fieldOfView,
	const repo::lib::RepoVector3D &lookAt,
	const repo::lib::RepoVector3D &position,
	const repo::lib::RepoVector3D &up,
	const std::string   &name,
	const int           &apiLevel)
{
	RepoBSONBuilder builder;

	//--------------------------------------------------------------------------
	// Compulsory fields such as _id, type, api as well as path
	// and optional name
	auto defaults = appendDefaults(REPO_NODE_TYPE_CAMERA, apiLevel, repo::lib::RepoUUID::createUUID(), name);
	builder.appendElements(defaults);

	//--------------------------------------------------------------------------
	// Aspect ratio
	builder.append(REPO_NODE_LABEL_ASPECT_RATIO, aspectRatio);

	//--------------------------------------------------------------------------
	// Far clipping plane
	builder.append(REPO_NODE_LABEL_FAR, farClippingPlane);

	//--------------------------------------------------------------------------
	// Near clipping plane
	builder.append(REPO_NODE_LABEL_NEAR, nearClippingPlane);

	//--------------------------------------------------------------------------
	// Field of view
	builder.append(REPO_NODE_LABEL_FOV, fieldOfView);

	//--------------------------------------------------------------------------
	// Look at vector
	builder.append(REPO_NODE_LABEL_LOOK_AT, lookAt);

	//--------------------------------------------------------------------------
	// Position vector
	builder.append(REPO_NODE_LABEL_POSITION, position);

	//--------------------------------------------------------------------------
	// Up vector
	builder.append(REPO_NODE_LABEL_UP, up);

	return CameraNode(builder.obj());
}

MaterialNode RepoBSONFactory::makeMaterialNode(
	const repo_material_t &material,
	const std::string     &name,
	const std::vector<repo::lib::RepoUUID> &parents,
	const int             &apiLevel)
{
	RepoBSONBuilder builder;

	// Compulsory fields such as _id, type, api as well as path
	// and optional name
	auto defaults = appendDefaults(REPO_NODE_TYPE_MATERIAL, apiLevel, repo::lib::RepoUUID::createUUID(), name, parents);
	builder.appendElements(defaults);

	if (material.ambient.size() > 0)
		builder.appendArray(REPO_NODE_MATERIAL_LABEL_AMBIENT, material.ambient);
	if (material.diffuse.size() > 0)
		builder.appendArray(REPO_NODE_MATERIAL_LABEL_DIFFUSE, material.diffuse);
	if (material.specular.size() > 0)
		builder.appendArray(REPO_NODE_MATERIAL_LABEL_SPECULAR, material.specular);
	if (material.emissive.size() > 0)
		builder.appendArray(REPO_NODE_MATERIAL_LABEL_EMISSIVE, material.emissive);

	if (material.isWireframe)
		builder.append(REPO_NODE_MATERIAL_LABEL_WIREFRAME, material.isWireframe);
	if (material.isTwoSided)
		builder.append(REPO_NODE_MATERIAL_LABEL_TWO_SIDED, material.isTwoSided);

	// Checking for NaN values
	if (material.opacity == material.opacity)
		builder.append(REPO_NODE_MATERIAL_LABEL_OPACITY, material.opacity);
	if (material.shininess == material.shininess)
		builder.append(REPO_NODE_MATERIAL_LABEL_SHININESS, material.shininess);
	if (material.shininessStrength == material.shininessStrength)
		builder.append(REPO_NODE_MATERIAL_LABEL_SHININESS_STRENGTH, material.shininessStrength);
	if (material.lineWeight == material.lineWeight)
		builder.append(REPO_NODE_MATERIAL_LABEL_LINE_WEIGHT, material.lineWeight);

	return MaterialNode(builder.obj());
}

MetadataNode RepoBSONFactory::makeMetaDataNode(
	RepoBSON			          &metadata,
	const std::string             &mimeType,
	const std::string             &name,
	const std::vector<repo::lib::RepoUUID>  &parents,
	const int                     &apiLevel)
{
	RepoBSONBuilder builder;

	// Compulsory fields such as _id, type, api as well as path
	// and optional name
	auto defaults = appendDefaults(REPO_NODE_TYPE_METADATA, apiLevel, repo::lib::RepoUUID::createUUID(), name, parents);
	builder.appendElements(defaults);

	//--------------------------------------------------------------------------
	// Media type
	if (!mimeType.empty())
		builder.append(REPO_LABEL_MEDIA_TYPE, mimeType);

	//--------------------------------------------------------------------------
	// Add metadata subobject
	if (!metadata.isEmpty())
		builder.append(REPO_NODE_LABEL_METADATA, metadata);

	return MetadataNode(builder.obj());
}

static bool keyCheck(const char &c)
{
	return c == '$' || c == '.';
}

static std::string sanitiseKey(const std::string &key)
{
	std::string cleanedKey(key);
	std::replace_if(cleanedKey.begin(), cleanedKey.end(), keyCheck, ':');
	return cleanedKey;
}

MetadataNode RepoBSONFactory::makeMetaDataNode(
	const std::vector<std::string>  &keys,
	const std::vector<std::string>  &values,
	const std::string               &name,
	const std::vector<repo::lib::RepoUUID>     &parents,
	const int                       &apiLevel)
{
	RepoBSONBuilder builder, metaBuilder;
	// Compulsory fields such as _id, type, api as well as path
	// and optional name
	auto defaults = appendDefaults(REPO_NODE_TYPE_METADATA, apiLevel, repo::lib::RepoUUID::createUUID(), name, parents);
	builder.appendElements(defaults);

	//check keys and values have the same sizes

	if (keys.size() != values.size())
	{
		repoWarning << "makeMetaDataNode: number of keys (" << keys.size()
			<< ") does not match the number of values(" << values.size() << ")!";
	}

	std::vector<std::string>::const_iterator kit = keys.begin();
	std::vector<std::string>::const_iterator vit = values.begin();
	for (; kit != keys.end() && vit != values.end(); ++kit, ++vit)
	{
		std::string key = sanitiseKey(*kit);
		std::string value = *vit;

		if (!key.empty() && !value.empty())
		{
			//Check if it is a number, if it is, store it as a number

			try {
				long long valueInt = boost::lexical_cast<long long>(value);
				metaBuilder.append(key, valueInt);
			}
			catch (boost::bad_lexical_cast &)
			{
				//not an int, try a double

				try {
					double valueFloat = boost::lexical_cast<double>(value);
					metaBuilder.append(key, valueFloat);
				}
				catch (boost::bad_lexical_cast &)
				{
					//not an int or float, store as string
					metaBuilder.append(key, value);
				}
			}
		}
	}

	builder.append(REPO_NODE_LABEL_METADATA, metaBuilder.obj());

	return MetadataNode(builder.obj());
}

MetadataNode RepoBSONFactory::makeMetaDataNode(
	const std::unordered_map<std::string, std::string>  &data,
	const std::string               &name,
	const std::vector<repo::lib::RepoUUID>     &parents,
	const int                       &apiLevel)
{
	RepoBSONBuilder builder, metaBuilder;
	// Compulsory fields such as _id, type, api as well as path
	// and optional name
	auto defaults = appendDefaults(REPO_NODE_TYPE_METADATA, apiLevel, repo::lib::RepoUUID::createUUID(), name, parents);
	builder.appendElements(defaults);

	for (const auto &entry : data) {
		std::string key = sanitiseKey(entry.first);
		std::string value = entry.second;

		if (!key.empty() && !value.empty())
		{
			//Check if it is a number, if it is, store it as a number

			try {
				long long valueInt = boost::lexical_cast<long long>(value);
				metaBuilder.append(key, valueInt);
			}
			catch (boost::bad_lexical_cast &)
			{
				//not an int, try a double

				try {
					double valueFloat = boost::lexical_cast<double>(value);
					metaBuilder.append(key, valueFloat);
				}
				catch (boost::bad_lexical_cast &)
				{
					//not an int or float, store as string
					metaBuilder.append(key, value);
				}
			}
		}
	}

	builder.append(REPO_NODE_LABEL_METADATA, metaBuilder.obj());

	return MetadataNode(builder.obj());
}

MeshNode RepoBSONFactory::makeMeshNode(
	const std::vector<repo::lib::RepoVector3D>                  &vertices,
	const std::vector<repo_face_t>                    &faces,
	const std::vector<repo::lib::RepoVector3D>                  &normals,
	const std::vector<std::vector<float>>             &boundingBox,
	const std::vector<std::vector<repo::lib::RepoVector2D>>   &uvChannels,
	const std::vector<repo_color4d_t>                 &colors,
	const std::vector<std::vector<float>>             &outline,
	const std::string                           &name,
	const std::vector<repo::lib::RepoUUID>      &parents,
	const int                                   &apiLevel)
{
	RepoBSONBuilder builder;
	uint64_t bytesize = 0; //track the (approximate) size to know when we need to offload to gridFS
	repo::lib::RepoUUID uniqueID = repo::lib::RepoUUID::createUUID();
	auto defaults = appendDefaults(REPO_NODE_TYPE_MESH, apiLevel, repo::lib::RepoUUID::createUUID(), name, parents, uniqueID);
	bytesize += defaults.objsize();
	builder.appendElements(defaults);

	if (!vertices.size() || !faces.size())
	{
		repoWarning << "Creating a mesh (" << defaults.getUUIDField(REPO_NODE_LABEL_ID) << ") with no vertices/faces!";
	}
	std::unordered_map<std::string, std::pair<std::string, std::vector<uint8_t>>> binMapping;

	if (boundingBox.size() > 0)
	{
		RepoBSONBuilder arrayBuilder;

		for (int i = 0; i < boundingBox.size(); i++)
		{
			arrayBuilder.appendArray(std::to_string(i), boundingBox[i]);
			bytesize += boundingBox[i].size() * sizeof(boundingBox[i][0]);
		}

		builder.appendArray(REPO_NODE_MESH_LABEL_BOUNDING_BOX, arrayBuilder.obj());
	}

	if (outline.size() > 0)
	{
		RepoBSONBuilder arrayBuilder;

		for (int i = 0; i < outline.size(); i++)
		{
			arrayBuilder.appendArray(boost::lexical_cast<std::string>(i), outline[i]);
			bytesize += outline[i].size() * sizeof(outline[i][0]);
		}

		builder.appendArray(REPO_NODE_MESH_LABEL_OUTLINE, arrayBuilder.obj());
	}

	/*
		* TODO: because mongo has a stupid internal limit of 64MB, we can't store everything in a BSON
		* There are 2 options
		* 1. store binaries in memory outside of the bson and put it into GRIDFS at the point of commit
		* 2. leave mongo's bson, use our own/exteral library that doesn't have this limit and database handle this at the point of commit
		* below uses option 1, but ideally we should be doing option 2.
		*/

	if (vertices.size() > 0)
	{
		uint64_t verticesByteCount = vertices.size() * sizeof(vertices[0]);

		if (verticesByteCount + bytesize >= REPO_BSON_MAX_BYTE_SIZE)
		{
			std::string bName = uniqueID.toString() + "_vertices";
			//inclusion of this binary exceeds the maximum, store separately
			binMapping[REPO_NODE_MESH_LABEL_VERTICES] =
				std::pair<std::string, std::vector<uint8_t>>(bName, std::vector<uint8_t>());
			binMapping[REPO_NODE_MESH_LABEL_VERTICES].second.resize(verticesByteCount); //uint8_t will ensure it is a byte addrressing
			memcpy(binMapping[REPO_NODE_MESH_LABEL_VERTICES].second.data(), &vertices[0], verticesByteCount);
			bytesize += sizeof(bName);
		}
		else
		{
			builder.appendBinary(
				REPO_NODE_MESH_LABEL_VERTICES,
				&vertices[0],
				vertices.size() * sizeof(vertices[0])
			);
			bytesize += verticesByteCount;
		}
	}

	if (faces.size() > 0)
	{
		builder.append(REPO_NODE_MESH_LABEL_FACES_COUNT, (uint32_t)(faces.size()));

		// In API LEVEL 1, faces are stored as
		// [n1, v1, v2, ..., n2, v1, v2...]
		std::vector<repo_face_t>::iterator faceIt;

		MeshNode::Primitive primitive = MeshNode::Primitive::UNKNOWN;

		std::vector<uint32_t> facesLevel1;
		for (auto &face : faces) {
			auto nIndices = face.size();
			if (!nIndices)
			{
				repoWarning << "number of indices in this face is 0!";
			}
			if (primitive == MeshNode::Primitive::UNKNOWN) // The primitive type is unknown, so attempt to infer it
			{
				if (nIndices == 2) {
					primitive = MeshNode::Primitive::LINES;
				}
				else if (nIndices == 3) {
					primitive = MeshNode::Primitive::TRIANGLES;
				}
				else // The primitive type is not one we support
				{
					repoWarning << "unsupported primitive type - only lines and triangles are supported but this face has " << nIndices << " indices!";
				}
			}
			else  // (otherwise check for consistency with the existing type)
			{
				if (nIndices != static_cast<int>(primitive))
				{
					repoWarning << "mixing different primitives within a mesh is not supported!";
				}
			}
			facesLevel1.push_back(nIndices);
			for (uint32_t ind = 0; ind < nIndices; ind++)
			{
				facesLevel1.push_back(face[ind]);
			}
		}

		builder.append(REPO_NODE_MESH_LABEL_PRIMITIVE, static_cast<int>(primitive));

		uint64_t facesByteCount = facesLevel1.size() * sizeof(facesLevel1[0]);

		if (facesByteCount + bytesize >= REPO_BSON_MAX_BYTE_SIZE)
		{
			std::string bName = uniqueID.toString() + "_faces";
			//inclusion of this binary exceeds the maximum, store separately
			binMapping[REPO_NODE_MESH_LABEL_FACES] =
				std::pair<std::string, std::vector<uint8_t>>(bName, std::vector<uint8_t>());
			binMapping[REPO_NODE_MESH_LABEL_FACES].second.resize(facesByteCount); //uint8_t will ensure it is a byte addrressing
			memcpy(binMapping[REPO_NODE_MESH_LABEL_FACES].second.data(), &facesLevel1[0], facesByteCount);

			bytesize += sizeof(bName);
		}
		else
		{
			builder.appendBinary(
				REPO_NODE_MESH_LABEL_FACES,
				&facesLevel1[0],
				facesLevel1.size() * sizeof(facesLevel1[0])
			);

			bytesize += facesByteCount;
		}
	}

	if (normals.size() > 0)
	{
		uint64_t normalsByteCount = normals.size() * sizeof(normals[0]);

		if (normalsByteCount + bytesize >= REPO_BSON_MAX_BYTE_SIZE)
		{
			std::string bName = uniqueID.toString() + "_normals";
			//inclusion of this binary exceeds the maximum, store separately
			binMapping[REPO_NODE_MESH_LABEL_NORMALS] =
				std::pair<std::string, std::vector<uint8_t>>(bName, std::vector<uint8_t>());
			binMapping[REPO_NODE_MESH_LABEL_NORMALS].second.resize(normalsByteCount); //uint8_t will ensure it is a byte addrressing
			memcpy(binMapping[REPO_NODE_MESH_LABEL_NORMALS].second.data(), &normals[0], normalsByteCount);

			bytesize += sizeof(bName);
		}
		else
		{
			builder.appendBinary(
				REPO_NODE_MESH_LABEL_NORMALS,
				&normals[0],
				normals.size() * sizeof(normals[0]));

			bytesize += normalsByteCount;
		}
	}

	//if (!vertexHash.empty())
	//{
	//	// TODO: Fix this call - needs to be fixed as int conversion is overloaded
	//	//builder << REPO_NODE_LABEL_SHA256 << (long unsigned int)(vertexHash);
	//}

	//--------------------------------------------------------------------------
	// Vertex colors
	if (colors.size())
	{
		uint64_t colorsByteCount = colors.size() * sizeof(colors[0]);

		if (colorsByteCount + bytesize >= REPO_BSON_MAX_BYTE_SIZE)
		{
			std::string bName = uniqueID.toString() + "_colors";
			//inclusion of this binary exceeds the maximum, store separately
			binMapping[REPO_NODE_MESH_LABEL_COLORS] =
				std::pair<std::string, std::vector<uint8_t>>(bName, std::vector<uint8_t>());
			binMapping[REPO_NODE_MESH_LABEL_COLORS].second.resize(colorsByteCount); //uint8_t will ensure it is a byte addrressing
			memcpy(binMapping[REPO_NODE_MESH_LABEL_COLORS].second.data(), &colors[0], colorsByteCount);

			bytesize += sizeof(bName);
		}
		else
		{
			builder.appendBinary(
				REPO_NODE_MESH_LABEL_COLORS,
				&colors[0],
				colors.size() * sizeof(colors[0]));
			bytesize += colorsByteCount;
		}
	}

	//--------------------------------------------------------------------------
	// UV channels
	if (uvChannels.size() > 0)
	{
		// Could be unsigned __int64 if BSON had such construct (the closest is only __int64)
		builder.append(REPO_NODE_MESH_LABEL_UV_CHANNELS_COUNT, (uint32_t)(uvChannels.size()));

		std::vector<repo::lib::RepoVector2D> concatenated;

		for (auto it = uvChannels.begin(); it != uvChannels.end(); ++it)
		{
			std::vector<repo::lib::RepoVector2D> channel = *it;

			std::vector<repo::lib::RepoVector2D>::iterator cit;
			for (cit = channel.begin(); cit != channel.end(); ++cit)
			{
				concatenated.push_back(*cit);
			}
		}

		uint64_t uvByteCount = concatenated.size() * sizeof(concatenated[0]);

		if (uvByteCount + bytesize >= REPO_BSON_MAX_BYTE_SIZE)
		{
			std::string bName = uniqueID.toString() + "_uv";
			//inclusion of this binary exceeds the maximum, store separately
			binMapping[REPO_NODE_MESH_LABEL_UV_CHANNELS] =
				std::pair<std::string, std::vector<uint8_t>>(bName, std::vector<uint8_t>());
			binMapping[REPO_NODE_MESH_LABEL_UV_CHANNELS].second.resize(uvByteCount); //uint8_t will ensure it is a byte addrressing
			memcpy(binMapping[REPO_NODE_MESH_LABEL_UV_CHANNELS].second.data(), &concatenated[0], uvByteCount);

			bytesize += sizeof(bName);
		}
		else
		{
			builder.appendBinary(
				REPO_NODE_MESH_LABEL_UV_CHANNELS,
				&concatenated[0],
				concatenated.size() * sizeof(concatenated[0]));

			bytesize += uvByteCount;
		}
	}

	return MeshNode(builder.obj(), binMapping);
}

RepoProjectSettings RepoBSONFactory::makeRepoProjectSettings(
	const std::string &uniqueProjectName,
	const std::string &owner,
	const bool        &isFederate,
	const std::string &type,
	const std::string &description,
	const double pinSize,
	const double avatarHeight,
	const double visibilityLimit,
	const double speed,
	const double zNear,
	const double zFar)
{
	RepoBSONBuilder builder;

	//--------------------------------------------------------------------------
	// Project name
	if (!uniqueProjectName.empty())
		builder.append(REPO_LABEL_ID, uniqueProjectName);

	//--------------------------------------------------------------------------
	// Owner
	if (!owner.empty())
		builder.append(REPO_LABEL_OWNER, owner);

	//--------------------------------------------------------------------------
	// Description
	if (!description.empty())
		builder.append(REPO_LABEL_DESCRIPTION, description);

	//--------------------------------------------------------------------------
	// Type
	if (!type.empty())
		builder.append(REPO_LABEL_TYPE, type);

	//--------------------------------------------------------------------------
	// federate
	if (isFederate)
		builder.append(REPO_PROJECT_SETTINGS_LABEL_IS_FEDERATION, isFederate);

	//--------------------------------------------------------------------------
	// Properties (embedded sub-bson)
	RepoBSONBuilder propertiesBuilder;
	if (pinSize != REPO_DEFAULT_PROJECT_PIN_SIZE)
		propertiesBuilder.append(REPO_LABEL_PIN_SIZE, pinSize);
	if (avatarHeight != REPO_DEFAULT_PROJECT_AVATAR_HEIGHT)
		propertiesBuilder.append(REPO_LABEL_AVATAR_HEIGHT, avatarHeight);
	if (visibilityLimit != REPO_DEFAULT_PROJECT_VISIBILITY_LIMIT)
		propertiesBuilder.append(REPO_LABEL_VISIBILITY_LIMIT, visibilityLimit);
	if (speed != REPO_DEFAULT_PROJECT_SPEED)
		propertiesBuilder.append(REPO_LABEL_SPEED, speed);
	if (zNear != REPO_DEFAULT_PROJECT_ZNEAR)
		propertiesBuilder.append(REPO_LABEL_ZNEAR, zNear);
	if (zFar != REPO_DEFAULT_PROJECT_ZFAR)
		propertiesBuilder.append(REPO_LABEL_ZFAR, zFar);
	RepoBSON propertiesBSON = propertiesBuilder.obj();
	if (propertiesBSON.isValid() && !propertiesBSON.isEmpty())
		builder.append(REPO_LABEL_PROPERTIES, propertiesBSON);

	//--------------------------------------------------------------------------
	// Add to the parent object
	return RepoProjectSettings(builder.obj());
}

RepoRole RepoBSONFactory::makeRepoRole(
	const std::string &roleName,
	const std::string &database,
	const std::vector<RepoPermission> &permissions,
	const RepoRole &oldRole
)
{
	RepoRole updatedOldRole = oldRole.cloneAndUpdatePermissions(permissions);
	return _makeRepoRole(roleName,
		database,
		updatedOldRole.getPrivileges(),
		updatedOldRole.getInheritedRoles());
}

RepoRole RepoBSONFactory::_makeRepoRole(
	const std::string &roleName,
	const std::string &database,
	const std::vector<RepoPrivilege> &privileges,
	const std::vector<std::pair<std::string, std::string> > &inheritedRoles
)
{
	RepoBSONBuilder builder;
	builder.append(REPO_LABEL_ID, database + "." + roleName);
	builder.append(REPO_ROLE_LABEL_ROLE, roleName);
	builder.append(REPO_ROLE_LABEL_DATABASE, database);

	//====== Add Privileges ========
	if (privileges.size() > 0)
	{
		RepoBSONBuilder privilegesBuilder;
		for (size_t i = 0; i < privileges.size(); ++i)
		{
			const auto &p = privileges[i];
			RepoBSONBuilder innerBsonBuilder, actionBuilder;
			RepoBSON resource = BSON(REPO_ROLE_LABEL_DATABASE << p.database << REPO_ROLE_LABEL_COLLECTION << p.collection);
			innerBsonBuilder.append(REPO_ROLE_LABEL_RESOURCE, resource);

			for (size_t aCount = 0; aCount < p.actions.size(); ++aCount)
			{
				actionBuilder.append(std::to_string(aCount), RepoRole::dbActionToString(p.actions[aCount]));
			}

			innerBsonBuilder.appendArray(REPO_ROLE_LABEL_ACTIONS, actionBuilder.obj());

			privilegesBuilder.append(std::to_string(i), innerBsonBuilder.obj());
		}
		builder.appendArray(REPO_ROLE_LABEL_PRIVILEGES, privilegesBuilder.obj());
	}
	else
	{
		repoDebug << "Creating a role with no privileges!";
	}

	//====== Add Inherited Roles ========

	if (inheritedRoles.size() > 0)
	{
		RepoBSONBuilder inheritedRolesBuilder;

		for (size_t i = 0; i < inheritedRoles.size(); ++i)
		{
			RepoBSON parentRole = BSON(
				REPO_ROLE_LABEL_ROLE << inheritedRoles[i].second
				<< REPO_ROLE_LABEL_DATABASE << inheritedRoles[i].first
			);

			inheritedRolesBuilder.append(std::to_string(i), parentRole);
		}

		builder.appendArray(REPO_ROLE_LABEL_INHERITED_ROLES, inheritedRolesBuilder.obj());
	}

	return RepoRole(builder.obj());
}

RepoRef RepoBSONFactory::makeRepoRef(
	const std::string &fileName,
	const RepoRef::RefType &type,
	const std::string &link,
	const uint32_t size) {
	repo::core::model::RepoBSONBuilder builder;
	builder.append(REPO_LABEL_ID, fileName);
	builder.append(REPO_REF_LABEL_TYPE, RepoRef::convertTypeAsString(type));
	builder.append(REPO_REF_LABEL_LINK, link);
	builder.append(REPO_REF_LABEL_SIZE, (unsigned int)size);
	return RepoRef(builder.obj());
}

RepoUser RepoBSONFactory::makeRepoUser(
	const std::string                           &userName,
	const std::string                           &password,
	const std::string                           &firstName,
	const std::string                           &lastName,
	const std::string                           &email,
	const std::list<std::pair<std::string, std::string>>   &roles,
	const std::list<std::pair<std::string, std::string>>   &apiKeys,
	const std::vector<char>                     &avatar)
{
	RepoBSONBuilder builder;
	RepoBSONBuilder customDataBuilder;

	builder.append(REPO_LABEL_ID, repo::lib::RepoUUID::createUUID());
	if (!userName.empty())
		builder.append(REPO_USER_LABEL_USER, userName);

	if (!password.empty())
	{
		RepoBSONBuilder credentialsBuilder;
		credentialsBuilder.append(REPO_USER_LABEL_CLEARTEXT, password);
		builder.append(REPO_USER_LABEL_CREDENTIALS, credentialsBuilder.obj());
	}

	if (!firstName.empty())
		customDataBuilder.append(REPO_USER_LABEL_FIRST_NAME, firstName);

	if (!lastName.empty())
		customDataBuilder.append(REPO_USER_LABEL_LAST_NAME, lastName);

	if (!email.empty())
		customDataBuilder.append(REPO_USER_LABEL_EMAIL, email);

	if (!apiKeys.empty())
		customDataBuilder.appendArrayPair(REPO_USER_LABEL_API_KEYS, apiKeys, REPO_USER_LABEL_LABEL, REPO_USER_LABEL_KEY);

	if (avatar.size())
	{
		RepoBSONBuilder avatarBuilder;
		avatarBuilder.appendBinary(REPO_LABEL_DATA, &avatar.at(0), sizeof(avatar.at(0))*avatar.size());
		customDataBuilder.append(REPO_LABEL_AVATAR, avatarBuilder.obj());
	}

	builder.append(REPO_USER_LABEL_CUSTOM_DATA, customDataBuilder.obj());

	if (roles.size())
		builder.appendArrayPair(REPO_USER_LABEL_ROLES, roles, REPO_USER_LABEL_DB, REPO_USER_LABEL_ROLE);

	return RepoUser(builder.obj());
}

RepoUnityAssets RepoBSONFactory::makeRepoUnityAssets(
	const repo::lib::RepoUUID                   &revisionID,
	const std::vector<std::string>              &assets,
	const std::string                           &database,
	const std::string                           &model,
	const std::vector<double>                   &offset,
	const std::vector<std::string>              &vrAssetFiles,
	const std::vector<std::string>              &iosAssetFiles,
	const std::vector<std::string>              &androidAssetFiles,
	const std::vector<std::string>              &jsonFiles)
{
	RepoBSONBuilder builder;

	builder.append(REPO_LABEL_ID, revisionID);

	if (assets.size())
		builder.appendArray(REPO_UNITY_ASSETS_LABEL_ASSETS, assets);

	if (!database.empty())
		builder.append(REPO_LABEL_DATABASE, database);

	if (!model.empty())
		builder.append(REPO_LABEL_MODEL, model);

	if (offset.size())
		builder.appendArray(REPO_UNITY_ASSETS_LABEL_OFFSET, offset);

	if (vrAssetFiles.size())
		builder.appendArray(REPO_UNITY_ASSETS_LABEL_VRASSETS, vrAssetFiles);

	if (iosAssetFiles.size())
		builder.appendArray(REPO_UNITY_ASSETS_LABEL_IOSASSETS, iosAssetFiles);

	if (androidAssetFiles.size())
		builder.appendArray(REPO_UNITY_ASSETS_LABEL_ANDROIDASSETS, androidAssetFiles);

	if (jsonFiles.size())
		builder.appendArray(REPO_UNITY_ASSETS_LABEL_JSONFILES, jsonFiles);

	return RepoUnityAssets(builder.obj());
}

ReferenceNode RepoBSONFactory::makeReferenceNode(
	const std::string &database,
	const std::string &project,
	const repo::lib::RepoUUID    &revisionID,
	const bool        &isUniqueID,
	const std::string &name,
	const int         &apiLevel)
{
	RepoBSONBuilder builder;
	std::string nodeName = name.empty() ? database + "." + project : name;

	auto defaults = appendDefaults(REPO_NODE_TYPE_REFERENCE, apiLevel, repo::lib::RepoUUID::createUUID(), nodeName);
	builder.appendElements(defaults);

	//--------------------------------------------------------------------------
	// Project owner (company or individual)
	if (!database.empty())
		builder.append(REPO_NODE_REFERENCE_LABEL_OWNER, database);

	//--------------------------------------------------------------------------
	// Project name
	if (!project.empty())
		builder.append(REPO_NODE_REFERENCE_LABEL_PROJECT, project);

	//--------------------------------------------------------------------------
	// Revision ID (specific revision if UID, branch if SID)
	builder.append(
		REPO_NODE_REFERENCE_LABEL_REVISION_ID,
		revisionID);

	//--------------------------------------------------------------------------
	// Unique set if the revisionID is UID, not set if SID (branch)
	if (isUniqueID)
		builder.append(REPO_NODE_REFERENCE_LABEL_UNIQUE, isUniqueID);

	return ReferenceNode(builder.obj());
}

RevisionNode RepoBSONFactory::makeRevisionNode(
	const std::string			   &user,
	const repo::lib::RepoUUID                 &branch,
	const repo::lib::RepoUUID                 &id,
	const std::vector<repo::lib::RepoUUID>    &currentNodes,
	const std::vector<std::string> &files,
	const std::vector<repo::lib::RepoUUID>    &parent,
	const std::vector<double>    &worldOffset,
	const std::string              &message,
	const std::string              &tag,
	const int                      &apiLevel
)
{
	RepoBSONBuilder builder;

	//--------------------------------------------------------------------------
	// Compulsory fields such as _id, type, api as well as path
	auto defaults = appendDefaults(REPO_NODE_TYPE_REVISION, apiLevel, branch, "", parent, id);
	builder.appendElements(defaults);

	//--------------------------------------------------------------------------
	// Author
	if (!user.empty())
		builder.append(REPO_NODE_REVISION_LABEL_AUTHOR, user);

	//--------------------------------------------------------------------------
	// Message
	if (!message.empty())
		builder.append(REPO_NODE_REVISION_LABEL_MESSAGE, message);

	//--------------------------------------------------------------------------
	// Tag
	if (!tag.empty())
		builder.append(REPO_NODE_REVISION_LABEL_TAG, tag);

	//--------------------------------------------------------------------------
	// Timestamp
	builder.appendTimeStamp(REPO_NODE_REVISION_LABEL_TIMESTAMP);

	//--------------------------------------------------------------------------

	// Current Unique IDs
	if (currentNodes.size() > 0)
		builder.appendArray(REPO_NODE_REVISION_LABEL_CURRENT_UNIQUE_IDS, currentNodes);

	//--------------------------------------------------------------------------
	// Shift for world coordinates
	if (worldOffset.size() > 0)
		builder.appendArray(REPO_NODE_REVISION_LABEL_WORLD_COORD_SHIFT, worldOffset);

	//--------------------------------------------------------------------------
	// original files references
	if (files.size() > 0)
	{
		std::string uniqueIDStr = id.toString();
		mongo::BSONObjBuilder arrbuilder;
		for (int i = 0; i < files.size(); ++i)
		{
			arrbuilder << std::to_string(i) << uniqueIDStr + files[i];
		}

		builder.appendArray(REPO_NODE_REVISION_LABEL_REF_FILE, arrbuilder.obj());
	}

	return RevisionNode(builder.obj());
}

TextureNode RepoBSONFactory::makeTextureNode(
	const std::string &name,
	const char        *data,
	const uint32_t    &byteCount,
	const uint32_t    &width,
	const uint32_t    &height,
	const std::vector<repo::lib::RepoUUID>& parentIDs,
	const int         &apiLevel)
{
	RepoBSONBuilder builder;
	repo::lib::RepoUUID uniqueID = repo::lib::RepoUUID::createUUID();
	std::unordered_map<std::string, std::pair<std::string, std::vector<uint8_t>>> binMapping;
	auto defaults = appendDefaults(REPO_NODE_TYPE_TEXTURE, apiLevel, uniqueID, name, parentIDs);
	builder.appendElements(defaults);
	//--------------------------------------------------------------------------
	// Width
	builder.append(REPO_LABEL_WIDTH, width);

	//--------------------------------------------------------------------------
	// Height
	builder.append(REPO_LABEL_HEIGHT, height);

	//--------------------------------------------------------------------------
	// Format TODO: replace format with MIME Type?
	if (!name.empty())
	{
		boost::filesystem::path file{ name };
		std::string ext = file.extension().string();
		if (!ext.empty())
			builder.append(REPO_NODE_LABEL_EXTENSION, ext.substr(1, ext.size()));
	}

	//--------------------------------------------------------------------------
	// Data
	if (data && byteCount) {
		std::string bName = uniqueID.toString() + "_data";
		//inclusion of this binary exceeds the maximum, store separately
		binMapping[REPO_LABEL_DATA] =
			std::pair<std::string, std::vector<uint8_t>>(bName, std::vector<uint8_t>());
		binMapping[REPO_LABEL_DATA].second.resize(byteCount); //uint8_t will ensure it is a byte addrressing
		memcpy(binMapping[REPO_LABEL_DATA].second.data(), data, byteCount);
	}
	else
	{
		repoWarning << " Creating a texture node with no texture!";
	}

	return TextureNode(builder.obj(), binMapping);
}

TransformationNode RepoBSONFactory::makeTransformationNode(
	const repo::lib::RepoMatrix &transMatrix,
	const std::string                     &name,
	const std::vector<repo::lib::RepoUUID>		  &parents,
	const int                             &apiLevel)
{
	RepoBSONBuilder builder;

	auto defaults = appendDefaults(REPO_NODE_TYPE_TRANSFORMATION, apiLevel, repo::lib::RepoUUID::createUUID(), name, parents);
	builder.appendElements(defaults);

	builder.append(REPO_NODE_LABEL_MATRIX, transMatrix);

	return TransformationNode(builder.obj());
}

RepoTask RepoBSONFactory::makeTask(
	const std::string &name,
	const uint64_t &startTime,
	const uint64_t &endTime,
	const repo::lib::RepoUUID &sequenceID,
	const std::unordered_map<std::string, std::string> &data,
	const std::vector<repo::lib::RepoUUID> &resources,
	const repo::lib::RepoUUID &parent,
	const repo::lib::RepoUUID &id
) {
	RepoBSONBuilder builder;
	builder.append(REPO_LABEL_ID, id);
	builder.append(REPO_TASK_LABEL_NAME, name);
	builder.append(REPO_TASK_LABEL_START, (long long)startTime);
	builder.append(REPO_TASK_LABEL_END, (long long)endTime);
	builder.append(REPO_TASK_LABEL_SEQ_ID, sequenceID);

	if (!parent.isDefaultValue())
		builder.append(REPO_TASK_LABEL_PARENT, parent);

	if (resources.size()) {
		RepoBSONBuilder resourceBuilder;
		resourceBuilder.appendArray(REPO_TASK_SHARED_IDS, resources);
		builder.append(REPO_TASK_LABEL_RESOURCES, resourceBuilder.obj());
	}

	std::vector<RepoBSON> metaEntries;
	for (const auto &entry : data) {
		std::string key = sanitiseKey(entry.first);
		std::string value = entry.second;

		if (!key.empty() && !value.empty())
		{
			RepoBSONBuilder metaBuilder;
			//Check if it is a number, if it is, store it as a number

			metaBuilder.append(REPO_TASK_META_KEY, key);
			try {
				long long valueInt = boost::lexical_cast<long long>(value);
				metaBuilder.append(REPO_TASK_META_VALUE, valueInt);
			}
			catch (boost::bad_lexical_cast &)
			{
				//not an int, try a double
				try {
					double valueFloat = boost::lexical_cast<double>(value);
					metaBuilder.append(REPO_TASK_META_VALUE, valueFloat);
				}
				catch (boost::bad_lexical_cast &)
				{
					//not an int or float, store as string
					metaBuilder.append(REPO_TASK_META_VALUE, value);
				}
			}

			metaEntries.push_back(metaBuilder.obj());
		}
	}

	builder.appendArray(REPO_TASK_LABEL_DATA, metaEntries);

	return builder.obj();
}

RepoSequence RepoBSONFactory::makeSequence(
	const std::vector<repo::core::model::RepoSequence::FrameData> &frameData,
	const std::string &name,
	const repo::lib::RepoUUID &id,
	const uint64_t firstFrame,
	const uint64_t lastFrame
) {
	RepoBSONBuilder builder;
	builder.append(REPO_LABEL_ID, id);
	builder.append(REPO_SEQUENCE_LABEL_NAME, name);
	builder.append(REPO_SEQUENCE_LABEL_START_DATE, (long long)firstFrame);
	builder.append(REPO_SEQUENCE_LABEL_END_DATE, (long long)lastFrame);

	std::vector<RepoBSON> frames;

	for (const auto &frameEntry : frameData) {
		RepoBSONBuilder bsonBuilder;
		bsonBuilder.append(REPO_SEQUENCE_LABEL_DATE, mongo::Date_t(frameEntry.timestamp * 1000));
		bsonBuilder.append(REPO_SEQUENCE_LABEL_STATE, frameEntry.ref);

		frames.push_back(bsonBuilder.obj());
	}

	builder.appendArray(REPO_SEQUENCE_LABEL_FRAMES, frames);

	return RepoSequence(builder.obj());
}