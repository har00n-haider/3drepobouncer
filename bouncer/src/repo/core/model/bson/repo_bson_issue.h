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
*  Issue BSON
*/

#pragma once
#include "repo_bson.h"

//------------------------------------------------------------------------------
//
// Fields specific to issues only
//
//------------------------------------------------------------------------------

#define REPO_ISSUE_LABEL_REV_ID "rev_id"
#define REPO_ISSUE_LABEL_NAME   "name"
#define REPO_ISSUE_LABEL_TOPIC  "topic_type"
#define REPO_ISSUE_LABEL_STATUS "status"
#define REPO_ISSUE_LABEL_VIEWPOINTS "viewpoints"
#define REPO_ISSUE_LABEL_OWNER "owner"
#define REPO_ISSUE_LABEL_DESC "desc"
#define REPO_ISSUE_LABEL_VP_GUID "guid"
#define REPO_ISSUE_LABEL_VP_UP "up"
#define REPO_ISSUE_LABEL_VP_POS "position"
#define REPO_ISSUE_LABEL_VP_LOOKAT "look_at"
#define REPO_ISSUE_LABEL_VP_VIEWDIR "view_dir"
#define REPO_ISSUE_LABEL_VP_RIGHT "right"
#define REPO_ISSUE_LABEL_VP_FOV "fov"
#define REPO_ISSUE_LABEL_VP_AR "aspect_ratio"
#define REPO_ISSUE_LABEL_VP_FAR "far"
#define REPO_ISSUE_LABEL_VP_NEAR "near"
#define REPO_ISSUE_LABEL_CREATED "created"

namespace repo {
	namespace core {
		namespace model {
			class REPO_API_EXPORT RepoIssue : public RepoBSON
			{
			public:

				RepoIssue();

				RepoIssue(RepoBSON bson) : RepoBSON(bson){}

				~RepoIssue();
			};
		}// end namespace model
	} // end namespace core
} // end namespace repo
