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
				* Get the position of the light
				* @return returns a vector of the position
				*/
				repo_vector_t getPosition() const;

				/**
				* Get the direction of the light
				* @return returns a vector of the position
				*/
				repo_vector_t getDirection() const;

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
				* Get the type of the node as enum
				* @return returns the type of light as enum
				*/
				virtual LightNode::LightType getLightType() const;

				/**
				* Get ambient colour from the light
				* @return returns ambient color (r,g,b)
				*/
				virtual repo_color3d_t getAmbientColor() const;

				/**
				* Get value of constant attenuation
				* @return return constant attenuation as a float
				*/
				float getConstantAttenuation() const;

				/**
				* Get value of linear attenuation
				* @return return linear attenuation as a float
				*/
				float getLinearAttenuation() const;

				/**
				* Get value of quadratic attenuation
				* @return return quadratic attenuation as a float
				*/
				float getQuadraticAttenuation() const;

				/**
				* Get diffuse colour from the light
				* @return returns ambient color (r,g,b)
				*/
				virtual repo_color3d_t getDiffuseColor() const;

				/**
				* Get specular colour from the light
				* @return returns ambient color (r,g,b)
				*/
				virtual repo_color3d_t getSpecularColor() const;

				/**
				* Get inner cone angle (radians)
				* @return returns the angle of the inner cone of a spot light, in radians
				*/
				virtual float getInnerConeAngle() const;

				/**
				* Get outer cone angle (radians)
				* @return returns the angle of the outer cone of a spot light, in radians
				*/
				virtual float getOuterConeAngle() const;

				/**
				* Get the exponent value for spot lights
				* @return returns the spot exponent
				*/
				virtual float getSpotExponent() const;

				/**
				* Return a string of the light type given the type as a enum value
				* @param type type as enum
				* @return returns string value of the light type
				*/
				static std::string getLightTypeAsString(const LightNode::LightType &type);

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
