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

#include "repo_node_light.h"
#include "repo_bson_builder.h"

#define _USE_MATH_DEFINES
#include <math.h>

using namespace repo::core::model;

LightNode::LightNode() :
RepoNode()
{
}

LightNode::LightNode(RepoBSON bson) :
RepoNode(bson)
{
}

LightNode::~LightNode()
{
}

RepoNode LightNode::cloneAndApplyTransformation(
	const std::vector<float> &matrix) const
{
	RepoBSONBuilder builder;
	if (hasField(REPO_LIGHT_NODE_LABEL_DIRECTION))
	{
		builder.append(REPO_LIGHT_NODE_LABEL_DIRECTION, multiplyMatVec(matrix, getDirection()));
	}

	if (hasField(REPO_LIGHT_NODE_LABEL_POSITION))
	{
		builder.append(REPO_LIGHT_NODE_LABEL_POSITION, multiplyMatVec(matrix, getPosition()));
	}

	return LightNode(builder.appendElementsUnique(*this));
}

repo_vector_t LightNode::getPosition() const
{
	repo_vector_t vec;
	if (hasField(REPO_LIGHT_NODE_LABEL_POSITION))
	{
		std::vector<float> floatArr = getFloatArray(REPO_LIGHT_NODE_LABEL_POSITION);
		if (floatArr.size() >= 3)
		{
			//repo_vector_t is effectively float[3]
			std::copy(floatArr.begin(), floatArr.begin() + 3, (float*)&vec);
		}
	}

	return vec;
}

repo_vector_t LightNode::getDirection() const
{
	repo_vector_t vec;
	if (hasField(REPO_LIGHT_NODE_LABEL_DIRECTION))
	{
		std::vector<float> floatArr = getFloatArray(REPO_LIGHT_NODE_LABEL_DIRECTION);
		if (floatArr.size() >= 3)
		{
			//repo_vector_t is effectively float[3]
			std::copy(floatArr.begin(), floatArr.begin() + 3, (float*)&vec);
		}
	}

	return vec;
}

std::string LightNode::getTypeAsString(
	const LightNode::LightType &type)
{
	std::string res;

	switch (type)
	{
	case LightNode::LightType::AMBIENT:
		res = "ambient";
		break;
	case LightNode::LightType::DIRECTIONAL:
		res = "directional";
		break;
	case LightNode::LightType::SPOT:
		res = "spot";
		break;
	case LightNode::LightType::POINT:
		res = "point";
		break;
	}
	return res;
}