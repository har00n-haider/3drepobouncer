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
* Camera Node
*/

#pragma once
#include "repo_node.h"

namespace repo {
	namespace core {
		namespace model {
			//------------------------------------------------------------------------------
			//
			// Fields specific to camera only
			//
			//------------------------------------------------------------------------------
#define REPO_LIGHT_NODE_LABEL_LIGHT_TYPE		"light type"
#define REPO_LIGHT_NODE_LABEL_POSITION		   "position"
#define REPO_LIGHT_NODE_LABEL_DIRECTION		   "direction"
#define REPO_LIGHT_NODE_LABEL_COLOR_AMBIENT	   "ambient color"
#define REPO_LIGHT_NODE_LABEL_COLOR_SPECULAR   "specular color"
#define REPO_LIGHT_NODE_LABEL_COLOR_DIFFUSE    "diffuse color"
#define REPO_LIGHT_NODE_LABEL_CONE_ANGLE_INNER "inner cone angle"
#define REPO_LIGHT_NODE_LABEL_CONE_ANGLE_OUTER "outer cone angle"
#define REPO_LIGHT_NODE_LABEL_ATTEN_CONSTANT   "attenuation constant"
#define REPO_LIGHT_NODE_LABEL_ATTEN_LINEAR     "attenuation linear"
#define REPO_LIGHT_NODE_LABEL_ATTEN_QUADRATIC  "attenuation quadratic"

			//------------------------------------------------------------------------------

			class REPO_API_EXPORT LightNode :public RepoNode
			{
			public:
				enum class LightType { UNKNOWN, POINT, SPOT, AMBIENT, DIRECTIONAL };

				/**
				* Default constructor
				*/
				LightNode();

				/**
				* Construct a LightNode from a RepoBSON object
				* @param RepoBSON object
				*/
				LightNode(RepoBSON bson);

				/**
				* Default deconstructor
				*/
				~LightNode();

				/**
				* Get the type of node
				* @return returns the type as a string
				*/
				virtual std::string getType() const
				{
					return REPO_NODE_TYPE_LIGHT;
				}

				/**
				* Get the type of node as an enum
				* @return returns type as enum.
				*/
				virtual NodeType getTypeAsEnum() const
				{
					return NodeType::LIGHT;
				}

				/**
				* Check if the node is position dependant.
				* i.e. if parent transformation is merged onto the node,
				* does the node requre to a transformation applied to it
				* e.g. meshes and cameras are position dependant, metadata isn't
				* Default behaviour is false. Position dependant child requires
				* override this function.
				* @return true if node is positionDependant.
				*/
				virtual bool positionDependant() { return true; }

				/**
				* Return a string of the light type given the type as a enum value
				* @param type type as enum
				* @return returns string value of the light type
				*/
				static std::string getTypeAsString(const LightNode::LightType &type);

				/*
				*	------------- Delusional modifiers --------------
				*   These are like "setters" but not. We are actually
				*   creating a new bson object with the changed field
				*/

				/**
				*  Create a new object with transformation applied to the node
				* default behaviour is do nothing. Children object
				* needs to override this function to perform their own specific behaviour.
				* @param matrix transformation matrix to apply.
				* @return returns a new object with transformation applied.
				*/
				virtual RepoNode cloneAndApplyTransformation(
					const std::vector<float> &matrix) const;
			};
		} //namespace model
	} //namespace core
} //namespace repo
