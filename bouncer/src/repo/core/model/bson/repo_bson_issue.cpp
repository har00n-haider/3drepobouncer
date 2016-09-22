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
*  User setting BSON
*/

#include "repo_bson_builder.h"
#include "repo_bson_issue.h"

using namespace repo::core::model;

RepoIssue::RepoIssue() : RepoBSON()
{
}

RepoIssue::~RepoIssue()
{
}

int64_t RepoIssue::getIssueNumber() const
{
	int64_t num = 0;
	if (hasField(REPO_ISSUE_LABEL_NUMBER))
	{
		num = getField(REPO_ISSUE_LABEL_NUMBER).Long();
	}

	return num;
}