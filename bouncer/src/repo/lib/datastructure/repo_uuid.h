/**
*  Copyright (C) 2016 3D Repo Ltd
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

#include <random>
#include <sstream>

namespace repo {
	namespace lib {

		static std::random_device              rd;
		static std::mt19937                    gen(rd());
		static std::uniform_int_distribution<> dis(0, 15);
		static std::uniform_int_distribution<> dis2(8, 11);

		class RepoUUID
		{
		public:

			static RepoUUID createUUID() 
			{
				return RepoUUID();
			};

			/**
			* Converts a RepoUUID to string
			* @return a string representation of repoUUID
			*/
			std::string toString() const 
			{
				std::stringstream ss;
				int i;
				ss << std::hex;
				for (i = 0; i < 8; i++) {
					ss << dis(gen);
				}
				ss << "-";
				for (i = 0; i < 4; i++) {
					ss << dis(gen);
				}
				ss << "-4";
				for (i = 0; i < 3; i++) {
					ss << dis(gen);
				}
				ss << "-";
				ss << dis2(gen);
				for (i = 0; i < 3; i++) {
					ss << dis(gen);
				}
				ss << "-";
				for (i = 0; i < 12; i++) {
					ss << dis(gen);
				};
				return ss.str();
				
			};
		};
	}
}
