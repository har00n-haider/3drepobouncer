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
* Controller that communicates to manipulator objects.
* Typically used by Desktop Client
* loosely following the model-view-controller design pattern
*/

#pragma once

#include <map>
#include <memory>
#include <string>
#include "repo_bouncer_global.h"


namespace repo {
	class REPO_API_EXPORT RepoController
	{
	public:

		class RepoToken;

		/**
			* Constructor
			* @param listeners a list of listeners subscribing to the log
			* @param numConcurrentOps maximum number of requests it can handle concurrently
			* @param numDBConn number of concurrent connections to the database
			*/
		RepoController();

		/**
			* Destructor
			*/
		~RepoController();

		void init();
	};
}
