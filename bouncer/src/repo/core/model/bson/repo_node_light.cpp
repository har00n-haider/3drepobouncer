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

LightNode::LightType LightNode::getLightType() const
{
	auto lightType = getStringField(REPO_LIGHT_NODE_LABEL_LIGHT_TYPE);
	if (lightType == "ambient") return LightNode::LightType::AMBIENT;
	if (lightType == "directional") return LightNode::LightType::DIRECTIONAL;
	if (lightType == "spot") return LightNode::LightType::SPOT;
	if (lightType == "point") return LightNode::LightType::POINT;
	repoWarning << "Unknown light type : " << lightType;
	return LightNode::LightType::UNKNOWN;
}

std::string LightNode::getLightTypeAsString(
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

repo_color3d_t LightNode::getAmbientColor() const
{
	repo_color3d_t color;
	auto am = getFloatArray(REPO_LIGHT_NODE_LABEL_COLOR_AMBIENT);
	if (am.size() >= 3)
	{
		color = { am[0], am[1], am[2] };
	}
	return color;
}

repo_color3d_t LightNode::getDiffuseColor() const
{
	repo_color3d_t color;
	auto di = getFloatArray(REPO_LIGHT_NODE_LABEL_COLOR_DIFFUSE);
	if (di.size() >= 3)
	{
		color = { di[0], di[1], di[2] };
	}
	return color;
}

repo_color3d_t LightNode::getSpecularColor() const
{
	repo_color3d_t color;
	auto sp = getFloatArray(REPO_LIGHT_NODE_LABEL_COLOR_SPECULAR);
	if (sp.size() >= 3)
	{
		color = { sp[0], sp[1], sp[2] };
	}
	return color;
}

float LightNode::getConstantAttenuation() const
{
	float res;
	if (hasField(REPO_LIGHT_NODE_LABEL_ATTEN_CONSTANT))
	{
		auto att = getField(REPO_LIGHT_NODE_LABEL_ATTEN_CONSTANT);
		if (att.type() == repo::core::model::ElementType::DOUBLE)
			res = att.Double();
		else
		{
			repoError << "Unable to retrieve " << REPO_LIGHT_NODE_LABEL_ATTEN_CONSTANT << ": Unexpected type";
		}
	}

	return res;
}

float LightNode::getLinearAttenuation() const
{
	float res;
	if (hasField(REPO_LIGHT_NODE_LABEL_ATTEN_LINEAR))
	{
		auto att = getField(REPO_LIGHT_NODE_LABEL_ATTEN_LINEAR);
		if (att.type() == repo::core::model::ElementType::DOUBLE)
			res = att.Double();
		else
		{
			repoError << "Unable to retrieve " << REPO_LIGHT_NODE_LABEL_ATTEN_LINEAR << ": Unexpected type";
		}
	}

	return res;
}

float LightNode::getQuadraticAttenuation() const
{
	float res;
	if (hasField(REPO_LIGHT_NODE_LABEL_ATTEN_QUADRATIC))
	{
		auto att = getField(REPO_LIGHT_NODE_LABEL_ATTEN_QUADRATIC);
		if (att.type() == repo::core::model::ElementType::DOUBLE)
			res = att.Double();
		else
		{
			repoError << "Unable to retrieve " << REPO_LIGHT_NODE_LABEL_ATTEN_QUADRATIC << ": Unexpected type";
		}
	}

	return res;
}

float LightNode::getInnerConeAngle() const
{
	float res;
	if (hasField(REPO_LIGHT_NODE_LABEL_CONE_ANGLE_INNER))
	{
		auto att = getField(REPO_LIGHT_NODE_LABEL_CONE_ANGLE_INNER);
		if (att.type() == repo::core::model::ElementType::DOUBLE)
			res = att.Double();
		else
		{
			repoError << "Unable to retrieve " << REPO_LIGHT_NODE_LABEL_CONE_ANGLE_INNER << ": Unexpected type";
		}
	}

	return res;
}

float LightNode::getOuterConeAngle() const
{
	float res;
	if (hasField(REPO_LIGHT_NODE_LABEL_CONE_ANGLE_OUTER))
	{
		auto att = getField(REPO_LIGHT_NODE_LABEL_CONE_ANGLE_OUTER);
		if (att.type() == repo::core::model::ElementType::DOUBLE)
			res = att.Double();
		else
		{
			repoError << "Unable to retrieve " << REPO_LIGHT_NODE_LABEL_CONE_ANGLE_OUTER << ": Unexpected type";
		}
	}

	return res;
}

float LightNode::getSpotExponent() const
{
	return 1. - (getInnerConeAngle() / getOuterConeAngle());
}