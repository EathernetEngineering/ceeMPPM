# CeeHealth
# Copyright (C) 2025 Chloe Eather
# 
# This program is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version.
# 
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
# more details.
# 
# You should have received a copy of the GNU General Public License along with
# this program. If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 4.0)

enable_testing()

if (TARGET gtest_main)
	message(STATUS "Found Google Test, building tests.")
elseif (CEE_GOOGLETEST_DIR)
	add_subdirectory(CEE_GOOGLETEST_DIR)
	if (NOT TARGET gtest_main)
		message(SEND_ERROR "Could not add googletest."
			"Are you sure ${CEE_GOOGLETEST_DIR} is correct?")
	endif()
elseif (CEE_FIND_GOOGLETEST)
	find_package(PkgConfig REQUIRED)
	find_package(GTest REQUIRED)
else()
	message(SEND_ERROR "Did not find googletest, tests will not be built."
		"If you have provided an external googletest set CEE_GOOGLETEST_DIR.")
endif()

# Use:
#
# add_cee_unittest(
# 	TARGET <name>
# 	SRCS <file1> [<file2> ...]
# 	[LIBS <lib1> [<lib2> ...]]
# 	[DEFINES <DEF_1> [DEF2 ...]]
# 	[INCLUDE_DIRS <dir1> [<dir2> ...]]
# 	[ENVIRONMENT <ENV_1> [<ENV_2> ...]]
# )

function(add_cee_unittest)
	set(one_value_args TARGET)
	set(multi_value_args SRCS LIBS ENVIRONMENT DEFINES INCLUDE_DIRS)
	cmake_parse_arguments(
		ARG "" "${one_value_args}" "${multi_value_args}" ${ARGN})
	set(target test_${ARG_TARGET})
	set(SRC_COPY ${ARG_SRCS})
	add_executable(${target} ${SRC_COPY})
	target_compile_definitions(${target} PUBLIC ${ARG_DEFINES})
	target_include_directories(${target} PRIVATE
		${gtest_SOURCE_DIR}/include
		${ARG_INCLUDE_DIRS}
	)
	target_link_libraries(${target} PRIVATE ${ARG_LIBS})
	if (TARGET gtest_main)
		target_link_libraries(${target} PRIVATE gtest_main)
	elseif (CEE_FIND_GOOGLETEST)
		target_link_libraries(${target} PRIVATE GTest::gtest_main)
	endif()
	set_target_properties(${target}
		PROPERTIES
		CXX_STANDARD 20
		CXX_STANDARD_REQUIRED ON)
	add_test(NAME cee_${target} COMMAND ${target})
	if (DEFINED ARG_ENVIRNOMENT)
		set_test_properties(cee_${target} PROPERTIES ENVIRONMENT ${ARG_ENVRIONMENT})
	endif()
	set_property(TARGET ${target} PROPERTY FOLDER "cee tests")
endfunction()

set(MPPMTest_SOURCES
	mppm_log.cpp
	mppm_file.cpp
	mppm_parse.cpp
)

add_cee_unittest(
	TARGET mppm_unit_tests
	SRCS ${MPPMTest_SOURCES}
	LIBS ceeMPPM
	INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/mppm ${CMAKE_BINARY_DIR}/mppm
)

