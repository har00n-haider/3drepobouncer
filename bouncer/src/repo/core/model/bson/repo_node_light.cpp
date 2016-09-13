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
	//RepoBSONBuilder builder;
	//if (hasField(REPO_LIGHT_NODE_LABEL_POSITION))
	//{
	//	builder.append(REPO_NODE_LABEL_LOOK_AT, multiplyMatVec(matrix, getLookAt()));
	//}

	//if (hasField(REPO_NODE_LABEL_POSITION))
	//{
	//	builder.append(REPO_NODE_LABEL_POSITION, multiplyMatVec(matrix, getPosition()));
	//}

	//if (hasField(REPO_NODE_LABEL_UP))
	//{
	//	builder.append(REPO_NODE_LABEL_UP, multiplyMatVec(matrix, getUp()));
	//}
	repoError << "Trying to apply transformation on a light. not yet supported.";

	return *this;// LightNode(builder.appendElementsUnique(*this));
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