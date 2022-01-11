#  Copyright (C) 2015 3D Repo Ltd
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU Affero General Public License as
#  published by the Free Software Foundation, either version 3 of the
#  License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Affero General Public License for more details.
#
#  You should have received a copy of the GNU Affero General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Code to automatically generate CMakeList.txt in sub directories
# This project compiles a library with source files span across multiple directories
# So this generates a CMakeList.txt for each sub directory within the src folder
# update the SOURCES and HEADERS variable, and include their child directory

include(CMakeDependentOption)

project("3drepobouncer")

cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -fPIC")
if(${APPLE})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -framework Foundation")
endif()

option (LICENSE_CHECK  "Build with a license check, see for details: https://github.com/3drepo/wiki/wiki/Licensing-components-of-3drepo.io " OFF)


#===================== CRYPTOLENS ============================

if (LICENSE_CHECK )
	find_package(CRYPTOLENS REQUIRED)
	# pulling required license configuration strings from user defined values
	set(LICENSE_RSA_PUB_KEY_MOD "" CACHE STRING "Licensing public token modulus")
	set(LICENSE_RSA_PUB_KEY_EXP "" CACHE STRING "Licensing public token exponent")
	set(LICENSE_AUTH_TOKEN "" CACHE STRING "Access token to communicate with licence server")
	set(LICENSE_PRODUCT_ID "" CACHE STRING "The product ID assigned in the licensing server")
	set(LICENSE_TIMEOUT_SECONDS "1800" CACHE STRING "Time after which an occupied seat is released")
	# setting up definitions to inject values into code
	add_definitions(-DREPO_LICENSE_CHECK)
	add_definitions(-DLICENSE_RSA_PUB_KEY_MOD="${LICENSE_RSA_PUB_KEY_MOD}")
	add_definitions(-DLICENSE_RSA_PUB_KEY_EXP="${LICENSE_RSA_PUB_KEY_EXP}")
	add_definitions(-DLICENSE_AUTH_TOKEN="${LICENSE_AUTH_TOKEN}")
	add_definitions(-DLICENSE_PRODUCT_ID=${LICENSE_PRODUCT_ID})
	add_definitions(-DLICENSE_TIMEOUT_SECONDS=${LICENSE_TIMEOUT_SECONDS})
endif()

#================COMPILE 3D REPO BOUNCER ==============
option (REPO_BUILD_WRAPPER "If the C# wrapper is built in addition to the library" ON)
cmake_dependent_option (REPO_BUILD_WRAPPER_TESTS "If the C# wrapper test suite is built" OFF "REPO_BUILD_WRAPPER" OFF)
option (REPO_BUILD_CLIENT "If the Command Line Client is built in addition to the library" ON)
option (REPO_BUILD_TOOLS "If the Command Line Tool is built in addition to the library" OFF)
option (REPO_BUILD_TESTS "If the test suite for the core bouncer logic is built in addition to the library" OFF)
option (REPO_NO_GZIP "If zlib is not compiled into boost" OFF)

add_definitions( -DWIN32_LEAN_AND_MEAN )

#bouncer library
add_subdirectory(bouncer)

#client exe
if (REPO_BUILD_CLIENT)
	add_subdirectory(client)
endif()

# C# wrapper
if (REPO_BUILD_WRAPPER)
	add_subdirectory(wrapper)
endif()




