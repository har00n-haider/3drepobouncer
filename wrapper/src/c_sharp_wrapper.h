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

#pragma once

#include <repo/repo_controller.h>
namespace repo{
	namespace lib{
		class CSharpWrapper
		{
		public:

			~CSharpWrapper();

			/**
			* Singleton class - get its instance
			*/
			static std::shared_ptr<CSharpWrapper> getInstance();

			/**
			* Connect to a mongo database, authenticate by the admin database
			* @param config configuration file path
			* @return returns true if successfully connected, false otherwise
			*/
			bool connect(
				const std::string &config);

			static std::shared_ptr<CSharpWrapper> wrapper;
		private:
			CSharpWrapper();
			repo::RepoController* controller;
		};
	}
}

