# CeeHealth
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

find_package(PkgConfig REQUIRED)

list(APPEND PROFILER_PRIVATE_INCLUDEDIRS /usr/include ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
list(APPEND PROFILER_PRIVATE_INCLUDEDIRS ${CMAKE_SOURCE_DIR}/external/tracy/public)
list(APPEND PROFILER_PUBLIC_INCLUDEDIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)
list(APPEND PROFILER_PUBLIC_INCLUDEDIRS ${CMAKE_SOURCE_DIR}/external/tracy/public/tracy)
list(APPEND PROFILER_LIBRARYDIRS /usr/lib)
list(APPEND PROFILER_LIBRARIES spdlog::spdlog)

if (USE_TRACY)
	list(APPEND GUI_LIBRARIES TracyClient)
endif()

list(APPEND PROFILER_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/profiler.cpp
)

list(APPEND PROFILER_PUBLIC_HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/include/cee/profiler/profiler.h
)

if (USE_TRACY)
	add_library(ceeProfiler SHARED ${PROFILER_SOURCES})
	target_include_directories(ceeProfiler PRIVATE ${PROFILER_PRIVATE_INCLUDEDIRS})
	target_include_directories(ceeProfiler PUBLIC ${PROFILER_PUBLIC_INCLUDEDIRS})
	target_link_directories(ceeProfiler PRIVATE ${PROFILER_LIBRARYDIRS})
	target_link_libraries(ceeProfiler PUBLIC ${PROFILER_LIBRARIES})
else()
	add_library(ceeProfiler INTERFACE)
	target_include_directories(ceeProfiler INTERFACE ${PROFILER_PUBLIC_INCLUDEDIRS})
	target_link_libraries(ceeProfiler INTERFACE ${PROFILER_LIBRARIES})
endif()

set_target_properties(ceeProfiler
	PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	PUBLIC_HEADER "${PROFILER_PUBLIC_HEADERS}")



