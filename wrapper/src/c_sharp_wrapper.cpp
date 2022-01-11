/**
*  Copyright (C) 2017 3D Repo Ltd
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


#include "c_sharp_wrapper.h"
#include <iostream>

using namespace repo::lib;

std::shared_ptr<CSharpWrapper> CSharpWrapper::wrapper = nullptr;

CSharpWrapper::CSharpWrapper()
	: controller(new repo::RepoController())
{
}

CSharpWrapper::~CSharpWrapper()
{
	if (controller)
	{
		delete controller;
	}

}

bool CSharpWrapper::connect(
	const std::string &config
)
{
	std::cout << "ran connect" << "\n";
	return true;
}


std::shared_ptr<CSharpWrapper> CSharpWrapper::getInstance()
{
	if (!wrapper) wrapper = std::shared_ptr<CSharpWrapper>(new CSharpWrapper());
	return wrapper;
}