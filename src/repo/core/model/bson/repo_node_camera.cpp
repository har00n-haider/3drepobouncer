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
*  Camera node
*/

#include "repo_node_camera.h"

using namespace repo::core::model::bson;

CameraNode::CameraNode() :
RepoNode()
{
}

CameraNode::CameraNode(RepoBSON bson) :
RepoNode(bson)
{

}

CameraNode::~CameraNode()
{
}

CameraNode* CameraNode::createCameraNode(
	const float         &aspectRatio,
	const float         &farClippingPlane,
	const float         &nearClippingPlane,
	const float         &fieldOfView,
	const repo_vector_t &lookAt,
	const repo_vector_t &position,
	const repo_vector_t &up,
	const int           &apiLevel,
	const std::string   &name)
{
	RepoBSONBuilder builder;

	//--------------------------------------------------------------------------
	// Compulsory fields such as _id, type, api as well as path
	// and optional name
	appendDefaults(builder, REPO_NODE_TYPE_CAMERA, apiLevel, generateUUID(), name);

	//--------------------------------------------------------------------------
	// Aspect ratio
	builder << REPO_NODE_LABEL_ASPECT_RATIO << aspectRatio;

	//--------------------------------------------------------------------------
	// Far clipping plane
	builder << REPO_NODE_LABEL_FAR << farClippingPlane;

	//--------------------------------------------------------------------------
	// Near clipping plane
	builder << REPO_NODE_LABEL_NEAR << nearClippingPlane;

	//--------------------------------------------------------------------------
	// Field of view
	builder << REPO_NODE_LABEL_FOV << fieldOfView;

	//--------------------------------------------------------------------------
	// Look at vector
	builder.appendVector(REPO_NODE_LABEL_LOOK_AT, lookAt);

	//--------------------------------------------------------------------------
	// Position vector 
	builder.appendVector(REPO_NODE_LABEL_POSITION, position);

	//--------------------------------------------------------------------------
	// Up vector
	builder.appendVector(REPO_NODE_LABEL_UP, up);

	return new CameraNode(builder.obj());
}

repo_vector_t CameraNode::getPosition() const
{
	repo_vector_t vec;
	if (hasField(REPO_NODE_LABEL_POSITION))
	{
		std::vector<float> floatArr = getFloatArray(REPO_NODE_LABEL_POSITION);
		if (floatArr.size() >= 3)
		{
			//repo_vector_t is effectively float[3]
			std::copy(floatArr.begin(), floatArr.begin() + 3, (float*) &vec);
		}
	}

	return vec;
}

repo_vector_t CameraNode::getLookAt() const
{
	repo_vector_t vec;
	if (hasField(REPO_NODE_LABEL_LOOK_AT))
	{
		std::vector<float> floatArr = getFloatArray(REPO_NODE_LABEL_LOOK_AT);
		if (floatArr.size() >= 3)
		{
			//repo_vector_t is effectively float[3]
			std::copy(floatArr.begin(), floatArr.begin() + 3, (float*)&vec);
		}
	}

	return vec;
}


repo_vector_t CameraNode::getUp() const
{
	repo_vector_t vec;
	if (hasField(REPO_NODE_LABEL_UP))
	{
		std::vector<float> floatArr = getFloatArray(REPO_NODE_LABEL_UP);
		if (floatArr.size() >= 3)
		{
			//repo_vector_t is effectively float[3]
			std::copy(floatArr.begin(), floatArr.begin() + 3, (float*) &vec);
		}
	}

	return vec;
}



std::vector<float> CameraNode::getCameraMatrix(
	const bool &rowMajor) const
{

	/** todo: test ... should work, but i'm not absolutely sure */

	std::vector<float> mat;
	mat.resize(16);

	uint8_t a1, a2, a3, a4, b1, b2, b3, b4, c1, c2, c3, c4, d1, d2, d3, d4;

	if (rowMajor)
	{
		//row is the fast dimension
		a1 = 0;
		a2 = 1;
		a3 = 2;
		a4 = 3;

		b1 = 4;
		b2 = 5;
		b3 = 6;
		b4 = 7;

		c1 = 8;
		c2 = 9;
		c3 = 10;
		c4 = 11;

		d1 = 12;
		d2 = 13;
		d3 = 14;
		d4 = 15;
	}
	else
	{
		//col is fast dimension
		a1 = 0;
		a2 = 4;
		a3 = 8;
		a4 = 12;

		b1 = 1;
		b2 = 5;
		b3 = 9;
		b4 = 13;

		c1 = 2;
		c2 = 6;
		c3 = 10;
		c4 = 14;

		d1 = 3;
		d2 = 7;
		d3 = 11;
		d4 = 15;
	}

	/** We don't know whether these vectors are already normalized ...*/
	repo_vector_t zaxis = getLookAt();  
	repo_vector_t yaxis = getUp();      
	repo_vector_t xaxis = crossProduct(yaxis, zaxis);  

	normalize(zaxis);
	normalize(yaxis);
	normalize(xaxis);

	repo_vector_t position = getPosition();

	mat[a4] = -dotProduct(xaxis, position);
	mat[b4] = -dotProduct(yaxis, position);
	mat[c4] = -dotProduct(zaxis, position);

	mat[a1] = xaxis.x;
	mat[a2] = xaxis.y;
	mat[a3] = xaxis.z;

	mat[b1] = yaxis.x;
	mat[b2] = yaxis.y;
	mat[b3] = yaxis.z;

	mat[c1] = zaxis.x;
	mat[c2] = zaxis.y;
	mat[c3] = zaxis.z;

	mat[d1] = mat[d2] = mat[d3] = 0.f;
	mat[d4] = 1.f;

	return mat;
}