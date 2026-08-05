# ceeCore
# Copyright (C) 2025 2026 Chloe Eather
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

project(ceeCore LANGUAGES CXX VERSION 1.0.0)

include(CMakeDependentOption)
include(GNUInstallDirs)

option(CEE_ENABLE_ASSERTIONS "Allow assertions at runtime" OFF)
cmake_dependent_option(CEE_ENABLE_ASSERTIONS_RAISE "Raise signal on assertion failure" ON CEE_ENABLE_ASSERTIONS OFF)

list(APPEND CEE_CORE_PRIVATE_INCLUDEDIRS /usr/include ${CMAKE_CURRENT_SOURCE_DIR})
list(APPEND CEE_CORE_PUBLIC_INCLUDEDIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_BINARY_DIR}/include)
list(APPEND CEE_CORE_LIBRARIES fmt::fmt spdlog::spdlog)

list(APPEND CEE_CORE_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/files.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/log.cpp
)

add_library(ceeCore ${CEE_CORE_SOURCES})

set_target_properties(ceeCore PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON)

target_include_directories(ceeCore PRIVATE ${CEE_CORE_PRIVATE_INCLUDEDIRS})
target_include_directories(ceeCore PUBLIC ${CEE_CORE_PUBLIC_INCLUDEDIRS})
target_link_libraries(ceeCore PUBLIC ${CEE_CORE_LIBRARIES})

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
		target_compile_options(ceeCore PRIVATE -Wall -Wextra -Wno-unused-parameter)
	if (CEE_USE_ASAN)
		target_compile_options(ceeCore PRIVATE -fsanitize=address,leak)
		target_link_options(ceeCore INTERFACE -fsanitize=address,leak)
	endif ()
else ()
	if (CEE_USE_ASAN)
		message(WARNING "Building ceeCore with address sanitizer in non-debug build")
		target_compile_options(ceeCore PRIVATE -fsanitize=address,leak)
		target_link_options(ceeCore INTERFACE -fsanitize=address,leak)
	endif ()
endif ()

configure_file(config.h.in "${CMAKE_CURRENT_BINARY_DIR}/include/cee/core/config.h" @ONLY)

