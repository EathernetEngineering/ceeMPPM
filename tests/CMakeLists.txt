# ceeMPPM
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
	target_link_libraries(${target} PRIVATE GTest::gtest_main)
	if (CEE_USE_ASAN)
		target_compile_options(${target} PRIVATE -fsanitize=address,leak)
		target_link_options(${target} PRIVATE -fsanitize=address,leak)
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

set(CORE_TEST_SOURCES
	core_file.cpp
)

set(FONT_TEST_SOURCES
	atlas_cache.cpp
	glyph_cache.cpp
	font_managment.cpp
)

add_cee_unittest(
	TARGET core_utils
	SRCS ${CORE_TEST_SOURCES}
	LIBS ceeMPPM
	INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/ceeCore
)

add_cee_unittest(
	TARGET font_lib
	SRCS ${FONT_TEST_SOURCES}
	LIBS ceeGUI
	INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/gui
)

