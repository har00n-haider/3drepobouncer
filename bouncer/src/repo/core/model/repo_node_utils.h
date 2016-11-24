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
* Static utility functions for nodes
*/

#pragma once
#include <iostream>
#include <algorithm>

#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <sstream>

#include "../../lib/repo_log.h"

//abstract out the use of boost inside the node codes
//incase we want to change it in the future
typedef boost::uuids::uuid repoUUID;

struct RepoUUIDHasher
{
	std::size_t operator()(const repoUUID& uid) const
	{
		return boost::hash<boost::uuids::uuid>()(uid);
	}
};

typedef struct{
	std::vector<float> ambient;
	std::vector<float> diffuse;
	std::vector<float> specular;
	std::vector<float> emissive;
	float opacity;
	float shininess;
	float shininessStrength;
	bool isWireframe;
	bool isTwoSided;
}repo_material_t;

typedef struct{
	float r;
	float g;
	float b;
	float a;
}repo_color4d_t;

typedef struct{
	float r;
	float g;
	float b;
}repo_color3d_t;

template <typename T>
struct _repo_vector{
	T x;
	T y;
	T z;
};


typedef struct{
	float x;
	float y;
}repo_vector2d_t;

using repo_double_vector_t = _repo_vector < double > ;
using repo_vector_t = _repo_vector < float >;

using repo_face_t =  std::vector<uint32_t>;

//This is used to map info for multipart optimization
typedef struct{
	repo_vector_t min;
	repo_vector_t max;
	repoUUID      mesh_id;
	repoUUID      material_id;
	int32_t       vertFrom;
	int32_t       vertTo;
	int32_t       triFrom;
	int32_t       triTo;
}repo_mesh_mapping_t;

static boost::uuids::random_generator gen;

static repoUUID generateUUID(){
	return gen();
}

//FIXME: scope this

/*!
* Returns a valid uuid representation of a given string. If empty, returns
* a randomly generated uuid. If the string is not a uuid representation,
* the string is hashed and appended with given suffix to prevent
* uuid clashes in cases where two objects such as a mesh and a
* transformation share the same name.
*
* \param text Can be any string including a valid UUID representation
*             without '{' and '}'.
* \param suffix Numerical suffix to prevent name clashes, eg "01".
* \return valid uuid
*/
static repoUUID stringToUUID(
	const std::string &text,
	const std::string &suffix = std::string())
{
	boost::uuids::uuid uuid;
	if (text.empty())
		uuid = generateUUID();
	else
	{
		try
		{
			boost::uuids::string_generator gen;
			if (text.substr(0, 1) != "{")
				uuid = gen("{" + text + "}");
			else
				uuid = gen(text);
		}
		catch (std::runtime_error e)
		{
			// uniformly distributed hash
			boost::hash<std::string> string_hash;
			std::string hashedUUID;
			std::stringstream str;
			str << string_hash(text);
			str >> hashedUUID;

			// uuid: 8 + 4 + 4 + 4 + 12 = 32
			// pad with zero, leave last places empty for suffix
			while (hashedUUID.size() < 32 - suffix.size())
				hashedUUID.append("0");
			hashedUUID.append(suffix);
			uuid = stringToUUID(hashedUUID, suffix);
		}
	}
	return uuid;
}

/**
* Converts a RepoUUID to string
* @param id repoUUID to convert
* @return a string representation of repoUUID
*/
static std::string UUIDtoString(const repoUUID &id)
{
	return boost::lexical_cast<std::string>(id);
}

static std::string toString(const repo_face_t &f)
{
	std::string str;
	unsigned int mNumIndices = f.size();

	str += "[";
	for (unsigned int i = 0; i < mNumIndices; i++)
	{
		str += std::to_string(f[i]);
		if (i != mNumIndices - 1)
			str += ", ";
	}
	str += "]";
	return str;
}

static std::string toString(const repo_color4d_t &color)
{
	std::stringstream sstr;
	sstr << "[" << color.r << ", " << color.g << ", " << color.b << ", " << color.a << "]";
	return sstr.str();
}

template <typename T>
static std::string toString(const _repo_vector<T> &vec)
{
	std::stringstream sstr;
	sstr << "[" << vec.x << ", " << vec.y << ", " << vec.z << "]";
	return sstr.str();
}

static std::string toString(const repo_vector2d_t &vec)
{
	std::stringstream sstr;
	sstr << "[" << vec.x << ", " << vec.y << "]";
	return sstr.str();
}

/**
* \brief Returns a compacted string representation of a given vector
* as [toString(0) ... toString(n)] where only the very first and the very
* last elements are displayed.
* \sa toString()
* @param vec vector to convert to string
* @return returns a string representing the vector
*/
template <class T>
static std::string vectorToString(const std::vector<T> &vec)
{
	{
		std::string str;
		if (vec.size() > 0)
		{
			str += "[" + toString(vec.at(0));
			if (vec.size() > 1)
				str += ", ..., " + toString(vec.at(vec.size() - 1));
			str += "]";
		}
		return str;
	}
}

template <typename T>
static T dotProduct(const _repo_vector<T> a, const _repo_vector<T> b)
{
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

template <typename T>
static _repo_vector<T> crossProduct(const _repo_vector<T> &a, const _repo_vector<T> &b)
{
	_repo_vector<T> product;
	product.x = (a.y * b.z) - (a.z * b.y);
	product.y = (a.z * b.x) - (a.x * b.z);
	product.z = (a.x * b.y) - (a.y * b.x);

	return product;
}

template <typename T>
static std::string printMat(const std::vector<T> &mat)
{
	std::stringstream ss;
	for (int i = 0; i < mat.size(); ++i)
	{
		ss << " " << mat[i];
		if (i % 4 == 3)
		{
			ss << "\n";
		}
	}

	return ss.str();
}

template <typename T>
static std::string printVec(const _repo_vector<T> &vec)
{
	std::stringstream ss;
	ss << "[ " << vec.x << ", " << vec.y << " ," << vec.z << " ]";

	return ss.str();
}

/**
* Matrix x vector multiplication
* NOTE: this assumes matrix has row as fast dimension!
* @param mat 4x4 matrix
* @param vec vector
* @return returns the resulting vector.
*/
template <typename T>
static _repo_vector<T> multiplyMatVec(const std::vector<T> &mat, const _repo_vector<T> &vec)
{
	_repo_vector<T> result;
	if (mat.size() != 16)
	{
		repoError << "Trying to perform a matrix x vector multiplation with unexpected matrix size(" << mat.size() << ")";
	}
	else{
		/*
			00 01 02 03
			04 05 06 07
			08 09 10 11
			12 13 14 15
			*/

		result.x = mat[0] * vec.x + mat[1] * vec.y + mat[2] * vec.z + mat[3];
		result.y = mat[4] * vec.x + mat[5] * vec.y + mat[6] * vec.z + mat[7];
		result.z = mat[8] * vec.x + mat[9] * vec.y + mat[10] * vec.z + mat[11];

		float sig = 1e-5;

		if (fabs(mat[12]) > sig || fabs(mat[13]) > sig || fabs(mat[14]) > sig || fabs(mat[15] - 1) > sig)
		{
			repoWarning << "Potentially incorrect transformation : does not expect the last row to have values!";
		}
	}

	return result;
}

/**
* Matrix x vector multiplication
* NOTE: this assumes matrix has row as fast dimension!
* This takes a 4x4 matrix but only use the 3x3 part.
* @param mat 4x4 matrix
* @param vec vector
* @return returns the resulting vector.
*/
template <typename T>
static _repo_vector<T> multiplyMatVecFake3x3(const std::vector<T> &mat, const _repo_vector<T> &vec)
{
	_repo_vector<T> result;
	if (mat.size() != 16)
	{
		repoError << "Trying to perform a matrix x vector multiplation with unexpected matrix size(" << mat.size() << ")";
	}
	else{
		/*
		00 01 02 03
		04 05 06 07
		08 09 10 11
		12 13 14 15
		*/

		result.x = mat[0] * vec.x + mat[1] * vec.y + mat[2] * vec.z;
		result.y = mat[4] * vec.x + mat[5] * vec.y + mat[6] * vec.z;
		result.z = mat[8] * vec.x + mat[9] * vec.y + mat[10] * vec.z;
	}

	return result;
}

template <typename T>
static float calculateDeterminant(std::vector<T> mat)
{
	/*
	00 01 02 03
	04 05 06 07
	08 09 10 11
	12 13 14 15
	*/

	T a1 = mat[0], a2 = mat[1], a3 = mat[2], a4 = mat[3];
	T b1 = mat[4], b2 = mat[5], b3 = mat[6], b4 = mat[7];
	T c1 = mat[8], c2 = mat[9], c3 = mat[10], c4 = mat[11];
	T d1 = mat[12], d2 = mat[13], d3 = mat[14], d4 = mat[15];

	T a1b2 = (a1 * b2) *(c3 * d4 - c4 * d3);
	T a1b3 = (a1 * b3) *(c4 * d2 - c2 * d4);
	T a1b4 = (a1 * b4) *(c2 * d3 - c3 * d2);

	T a2b1 = -(a2 * b1) *(c3 * d4 - c4 * d3);
	T a2b3 = -(a2 * b3) *(c4 * d1 - c1 * d4);
	T a2b4 = -(a2 * b4) *(c1 * d3 - c3 * d1);

	T a3b1 = (a3 * b1) *(c2 * d4 - c4 * d2);
	T a3b2 = (a3 * b2) *(c4 * d1 - c1 * d4);
	T a3b4 = (a3 * b4) *(c1 * d2 - c2 * d1);

	T a4b1 = -(a4 * b1) *(c2 * d3 - c3 * d2);
	T a4b2 = -(a4 * b2) *(c3 * d1 - c1 * d3);
	T a4b3 = -(a4 * b3) *(c1 * d2 - c2 * d1);

	return a1b2 + a1b3 + a1b4
		+ a2b1 + a2b3 + a2b4
		+ a3b1 + a3b2 + a3b4
		+ a4b1 + a4b2 + a4b3;
}

template <typename T>
static std::vector<T> invertMat(const std::vector<T> &mat)
{
	std::vector<T> result;
	result.resize(16);

	if (mat.size() != 16)
	{
		repoError << "Unsupported vector size (" << mat.size() << ")!";
	}
	else
	{
		const T det = calculateDeterminant(mat);
		if (det == 0)
		{
			repoError << "Trying to invert a matrix with determinant = 0!";
		}
		else
		{
			const T inv_det = 1. / det;

			T a1 = mat[0], a2 = mat[1], a3 = mat[2], a4 = mat[3];
			T b1 = mat[4], b2 = mat[5], b3 = mat[6], b4 = mat[7];
			T c1 = mat[8], c2 = mat[9], c3 = mat[10], c4 = mat[11];
			T d1 = mat[12], d2 = mat[13], d3 = mat[14], d4 = mat[15];

			result[0] = inv_det * (b2 * (c3 * d4 - c4 * d3) + b3 * (c4 * d2 - c2 * d4) + b4 * (c2 * d3 - c3 * d2));
			result[1] = -inv_det * (a2 * (c3 * d4 - c4 * d3) + a3 * (c4 * d2 - c2 * d4) + a4 * (c2 * d3 - c3 * d2));
			result[2] = inv_det * (a2 * (b3 * d4 - b4 * d3) + a3 * (b4 * d2 - b2 * d4) + a4 * (b2 * d3 - b3 * d2));
			result[3] = -inv_det * (a2 * (b3 * c4 - b4 * c3) + a3 * (b4 * c2 - b2 * c4) + a4 * (b2 * c3 - b3 * c2));

			result[4] = -inv_det * (b1 * (c3 * d4 - c4 * d3) + b3 * (c4 * d1 - c1 * d4) + b4 * (c1 * d3 - c3 * d1));
			result[5] = inv_det * (a1 * (c3 * d4 - c4 * d3) + a3 * (c4 * d1 - c1 * d4) + a4 * (c1 * d3 - c3 * d1));
			result[6] = -inv_det * (a1 * (b3 * d4 - b4 * d3) + a3 * (b4 * d1 - b1 * d4) + a4 * (b1 * d3 - b3 * d1));
			result[7] = inv_det * (a1 * (b3 * c4 - b4 * c3) + a3 * (b4 * c1 - b1 * c4) + a4 * (b1 * c3 - b3 * c1));

			result[8] = inv_det * (b1 * (c2 * d4 - c4 * d2) + b2 * (c4 * d1 - c1 * d4) + b4 * (c1 * d2 - c2 * d1));
			result[9] = -inv_det * (a1 * (c2 * d4 - c4 * d2) + a2 * (c4 * d1 - c1 * d4) + a4 * (c1 * d2 - c2 * d1));
			result[10] = inv_det * (a1 * (b2 * d4 - b4 * d2) + a2 * (b4 * d1 - b1 * d4) + a4 * (b1 * d2 - b2 * d1));
			result[11] = -inv_det * (a1 * (b2 * c4 - b4 * c2) + a2 * (b4 * c1 - b1 * c4) + a4 * (b1 * c2 - b2 * c1));

			result[12] = -inv_det * (b1 * (c2 * d3 - c3 * d2) + b2 * (c3 * d1 - c1 * d3) + b3 * (c1 * d2 - c2 * d1));
			result[13] = inv_det * (a1 * (c2 * d3 - c3 * d2) + a2 * (c3 * d1 - c1 * d3) + a3 * (c1 * d2 - c2 * d1));
			result[14] = -inv_det * (a1 * (b2 * d3 - b3 * d2) + a2 * (b3 * d1 - b1 * d3) + a3 * (b1 * d2 - b2 * d1));
			result[15] = inv_det * (a1 * (b2 * c3 - b3 * c2) + a2 * (b3 * c1 - b1 * c3) + a3 * (b1 * c2 - b2 * c1));
		}
	}

	return result;
}

template <typename T>
static std::vector<T> matMult(const std::vector<T> &mat1, const std::vector<T> &mat2)
{
	std::vector<T> result;
	if ((mat1.size() == mat2.size()) && mat1.size() == 16)
	{
		result.resize(16);

		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				size_t resultIdx = i * 4 + j;
				result[resultIdx] = 0;
				for (int k = 0; k < 4; ++k)
				{
					result[resultIdx] += mat1[i * 4 + k] * mat2[k * 4 + j];
				}
			}
		}
	}
	else
	{
		repoError << "We currently only support 4x4 matrix multiplications";
	}

	return result;
}

template <typename T>
static std::vector<T> transposeMat(const std::vector<T> &mat)
{
	std::vector<T> result(mat.begin(), mat.end());

	if (mat.size() != 16)
	{
		repoError << "Unsupported vector size (" << mat.size() << ")!";
	}
	else
	{
		/*
		00 01 02 03             00 04 08 12
		04 05 06 07   ----->    01 05 09 13
		08 09 10 11             02 06 10 14
		12 13 14 15             03 07 11 15
		*/

		result[1] = mat[4];
		result[4] = mat[1];
		result[2] = mat[8];
		result[8] = mat[2];
		result[3] = mat[12];
		result[12] = mat[3];
		result[6] = mat[9];
		result[9] = mat[6];
		result[7] = mat[13];
		result[13] = mat[7];
		result[11] = mat[14];
		result[14] = mat[11];
	}

	return result;
}

template <typename T>
static void normalize(_repo_vector<T> &a)
{
	float length = std::sqrt(a.x*a.x + a.y*a.y + a.z*a.z);

	if (length > 0)
	{
		a.x /= length;
		a.y /= length;
		a.z /= length;
	}
}

static bool nameCheck(const char &c)
{
	return c == ' ' || c == '$' || c == '.';
}

static bool dbNameCheck(const char &c)
{
	return c == '/' || c == '\\' || c == '.' || c == ' '
		|| c == '\"' || c == '$' || c == '*' || c == '<'
		|| c == '>' || c == ':' || c == '?' || c == '|';
}

static bool extNameCheck(const char &c)
{
	return c == ' ' || c == '$';
}

static std::string sanitizeExt(const std::string& name)
{
	// http://docs.mongodb.org/manual/reference/limits/#Restriction-on-Collection-Names
	std::string newName(name);
	std::replace_if(newName.begin(), newName.end(), extNameCheck, '_');

	return newName;
}

static std::string sanitizeName(const std::string& name)
{
	// http://docs.mongodb.org/manual/reference/limits/#Restriction-on-Collection-Names
	std::string newName(name);
	std::replace_if(newName.begin(), newName.end(), nameCheck, '_');

	return newName;
}

static std::string sanitizeDatabaseName(const std::string& name)
{
	// http://docs.mongodb.org/manual/reference/limits/#naming-restrictions

	// Cannot contain any of /\. "$*<>:|?
	std::string newName(name);
	std::replace_if(newName.begin(), newName.end(), dbNameCheck, '_');

	return newName;
}
