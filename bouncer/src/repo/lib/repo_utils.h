/**
*  Copyright (C) 2019 3D Repo Ltd
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

#include <boost/filesystem.hpp>
#include <string>
#include <boost/date_time/local_time/local_time.hpp>
#include <repo/lib/repo_utils.h>
#include <repo/lib/repo_exception.h>

namespace repo {
	namespace lib {
		static bool doesFileExist(const boost::filesystem::path& inputPath)
		{
			return boost::filesystem::exists(inputPath) && boost::filesystem::is_regular_file(inputPath);
		}

		static bool doesDirExist(const boost::filesystem::path& inputPath)
		{
			return boost::filesystem::exists(inputPath) && boost::filesystem::is_directory(inputPath);
		}

		static std::string getExtension(const boost::filesystem::path &inputPath) {
			return inputPath.extension().string();
		}

		static void toLower(std::string &str) {
			std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
		}

		static void toUpper(std::string &str) {
			std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::toupper(c); });
		}

		static std::string getEnvString(std::string const& envVarName)
		{
			char* value = getenv(envVarName.c_str());
			return (value && strlen(value) > 0) ? value : "";
		}

		static time_t timeZoneEpochToUtcEpoch(std::string const & inputTimeZoneName, time_t inputEpochSec)
		{
			// load database once
			static boost::local_time::tz_database timeZoneDatabase;
			if(timeZoneDatabase.region_list().size() == 0)
			{
				const std::string timeZoneDatabasePath = getEnvString("BOOST_TIMEZONE_CSV");
				if(timeZoneDatabasePath.length() == 0)
				{
					throw repo::lib::RepoException("Cannot find boost time zone database csv path");
				}
				timeZoneDatabase.load_from_file(timeZoneDatabasePath);
			}
			// need to check here first as boost will assert otherwise
			auto timeZoneList = timeZoneDatabase.region_list();
			bool timeZoneInDatabase = 
				std::find(
					timeZoneList.begin(),
					timeZoneList.end(),
					inputTimeZoneName) != 
				timeZoneList.end();
			if (!timeZoneInDatabase)
			{
				throw repo::lib::RepoException("Cannot find time zone in boost database");
			}
			// create a boost time zone
			boost::local_time::time_zone_ptr localTimeZone = 
				timeZoneDatabase.time_zone_from_region(inputTimeZoneName);
			// create a date time like object to represent the input epoch 
			boost::posix_time::ptime localEpoch = boost::posix_time::from_time_t(inputEpochSec);
			// create the local date time object with input epoch, quoted in input time zone
			boost::local_time::local_date_time localDateTime(
				localEpoch.date(), 
				localEpoch.time_of_day(), 
				localTimeZone, 
				boost::local_time::local_date_time::DST_CALC_OPTIONS::EXCEPTION_ON_ERROR);
			return boost::posix_time::to_time_t(localDateTime.utc_time());
		}
	}
}