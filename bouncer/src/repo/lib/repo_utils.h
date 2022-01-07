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

		static time_t timeZoneEpochToUtcEpoch(std::string const & timeZoneName, time_t timeZoneEpochSeconds)
		{
			// load database
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
			auto list = timeZoneDatabase.region_list();
			// create a boost time zone
			boost::local_time::time_zone_ptr localTimeZone = timeZoneDatabase.time_zone_from_region(timeZoneName);
			// create a boost ptime to represent the input time as unix epoch 
			boost::posix_time::ptime localEpoch = boost::posix_time::from_time_t(timeZoneEpochSeconds);
			// create the local date time object to corrected utc time 
			boost::local_time::local_date_time localDateTime{ localEpoch, localTimeZone };
			//TODO: remove this debug log
			std::cout << "UTC   time: "<< localDateTime.utc_time() << '\n';
			std::cout << "local time: "<< localDateTime.local_time() << '\n';
			std::cout << "local time zone name: " << localDateTime.zone_name() << '\n';
			std::cout << "UTC offset in time zone: " << localTimeZone->base_utc_offset() << std::endl;
			std::cout << "orig epoch: " << static_cast<time_t>(timeZoneEpochSeconds) << "\n";
			std::cout << "conv epoch: " << boost::posix_time::to_time_t(localDateTime.local_time()) << "\n";
			return boost::posix_time::to_time_t(localDateTime.local_time());
		}
	}
}