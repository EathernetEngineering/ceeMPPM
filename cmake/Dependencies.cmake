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

set(CEE_FMT_SOURCE_DIR "" CACHE PATH "Path to a fmt source tree")
set(CEE_GLM_SOURCE_DIR "" CACHE PATH "Path to a glm source tree")
set(CEE_GTEST_SOURCE_DIR "" CACHE PATH "Path to a GoogleTest source tree")
set(CEE_SPDLOG_SOURCE_DIR "" CACHE PATH "Path to a spdlog source tree")
set(CEE_TRACY_SOURCE_DIR "" CACHE PATH "Path to a tracy source tree")

find_package(PkgConfig)

if (NOT TARGET fmt::fmt)
	if (CEE_FMT_SOURCE_DIR)
		if (NOT IS_DIRECTORY "${CEE_FMT_SOURCE_DIR}")
			message(FATAL_ERROR "CEE_FMT_SOURCE_DIR is not a directory:\n"
				"  ${CEE_FMT_SOURCE_DIR}")
		endif ()
		if (NOT EXISTS "${CEE_FMT_SOURCE_DIR}/CMakeLists.txt")
			message(FATAL_ERROR "The provided fmt directory does not look like a source tree:\n"
				"  ${CEE_FMT_SOURCE_DIR}")
		endif ()

		message(STATUS "Building fmt from: ${CEE_FMT_SOURCE_DIR}")
		add_subdirectory("${CEE_FMT_SOURCE_DIR}"
			"${CMAKE_BINARY_DIR}/_deps/fmt"
			EXCLUDE_FROM_ALL)
	else ()
		find_package(fmt CONFIG QUIET)
		if (fmt_FOUND)
			message(STATUS "Found fmt")
		else ()
			message(FATAL_ERROR "fmt was not found.\n\n"
				"Install fmt or provide source with -DCEE_FMT_SOURCE_DIR=/path/to/fmt")
		endif ()
	endif()
endif (NOT TARGET fmt::fmt)

if (NOT TARGET glm::glm)
	if (CEE_GLM_SOURCE_DIR)
		if (NOT IS_DIRECTORY "${CEE_GLM_SOURCE_DIR}")
			message(FATAL_ERROR "CEE_GLM_SOURCE_DIR is not a directory:\n"
				"  ${CEE_GLM_SOURCE_DIR}")
		endif ()
		if (NOT EXISTS "${CEE_GLM_SOURCE_DIR}/CMakeLists.txt")
			message(FATAL_ERROR "The provided glm directory does not look like a source tree:\n"
				"  ${CEE_GLM_SOURCE_DIR}")
		endif ()

		message(STATUS "Building glm from: ${CEE_GLM_SOURCE_DIR}")
		add_subdirectory("${CEE_GLM_SOURCE_DIR}"
			"${CMAKE_BINARY_DIR}/_deps/glm"
			EXCLUDE_FROM_ALL)
	else ()
		find_package(GLM CONFIG QUIET)
		if (GLM_FOUND)
			message(STATUS "Found glm")
		else ()
			message(FATAL_ERROR "glm was not found.\n\n"
				"Install glm or provide source with -DCEE_GLM_SOURCE_DIR=/path/to/glm")
		endif ()
	endif ()
endif (NOT TARGET glm::glm)

if (NOT TARGET GTest::gtest)
	if (CEE_GTEST_SOURCE_DIR)
		if (NOT IS_DIRECTORY "${CEE_GTEST_SOURCE_DIR}")
			message(FATAL_ERROR "CEE_GTEST_SOURCE_DIR is not a directory:\n"
				"  ${CEE_GTEST_SOURCE_DIR}")
		endif ()
		if (NOT EXISTS "${CEE_GTEST_SOURCE_DIR}/CMakeLists.txt")
			message(FATAL_ERROR "The provided GoogleTest directory does not look like a source tree:\n"
				"  ${CEE_GTEST_SOURCE_DIR}")
		endif ()

		message(STATUS "Building GooogleTest from: ${CEE_GTEST_SOURCE_DIR}")
		
		set(BUILD_GMOCK OFF)
		set(GTEST_HAS_ABSL OFF)
		add_subdirectory("${CEE_GTEST_SOURCE_DIR}"
			"${CMAKE_BINARY_DIR}/_deps/GTest"
			EXCLUDE_FROM_ALL)
	else ()
		find_package(GTest CONFIG QUIET)
		if  (GTest_FOUND)
			message(STATUS "Found GoogleTest")
		else ()
			message(FATAL_ERROR "GooogleTest was not found.\n\n"
				"Install GoogleTest or provide source with -DCEE_GTEST_SOURCE_DIR=/path/to/googletest")
		endif ()
	endif ()
endif (NOT TARGET GTest::gtest)

if (NOT TARGET spdlog::spdlog)
	if (CEE_SPDLOG_SOURCE_DIR)
		if (NOT IS_DIRECTORY "${CEE_SPDLOG_SOURCE_DIR}")
			message(FATAL_ERROR "CEE_SPDLOG_SOURCE_DIR is not a directory:\n"
				"  ${CEE_SPDLOG_SOURCE_DIR}")
		endif ()
		if (NOT EXISTS "${CEE_GTEST_SOURCE_DIR}/CMakeLists.txt")
			message(FATAL_ERROR "The provided spdlog directory does not look like a source tree:\n"
				"  ${CEE_SPDLOG_SOURCE_DIR}")
		endif()

		message(STATUS "Building spdlog from: ${CEE_SPDLOG_SOURCE_DIR}")

		set(SPDLOG_BUILD_PIC ON)
		set(SPDLOG_ENABLE_PCH ON)
		set(SPDLOG_FMT_EXTERNAL ON)
		add_subdirectory("${CEE_SPDLOG_SOURCE_DIR}"
			"${CMAKE_BINARY_DIR}/_deps/spdlog"
			EXCLUDE_FROM_ALL)
	else ()
		find_package(spdlog CONFIG QUIET)
		if (spdlog_FOUND)
			message(STATUS "Found spdlog")
		else ()
			message(FATAL_ERROR "spdlog was not found.\n\n"
				"Install spdlog or provide source with -DCEE_SPDLOG_SOURCE_DIR=/path/to/spdlog")
		endif ()
	endif ()
endif (NOT TARGET spdlog::spdlog)

if (CEE_USE_TRACY AND NOT TARGET Tracy::TracyClient)
	if (CEE_TRACY_SOURCE_DIR)
		if (NOT IS_DIRECTORY "${CEE_TRACY_SOURCE_DIR}")
			message(FATAL_ERROR "CEE_TRACY_SOURCE_DIR is not a directory:\n"
				"  ${CEE_TRACY_SOURCE_DIR}")
		endif ()
		if (NOT EXISTS "${CEE_TRACY_SOURCE_DIR}/CMakeLists.txt")
			message(FATAL_ERROR "The provided Tracy directory does not look like a source tree:\n"
				"  ${CEE_TRACY_SOURCE_DIR}")
		endif ()

		message(STATUS "Building Tracy from: ${CEE_TRACY_SOURCE_DIR}")

		add_subdirectory("${CEE_TRACY_SOURCE_DIR}"
			"${CMAKE_BINARY_DIR}/_deps/Tracy"
			EXCLUDE_FROM_ALL)
		if (CEE_BUILD_TRACY_PROFILER)
			add_subdirectory("${CEE_TRACY_SOURCE_DIR}/profiler"
				"${CMAKE_BINARY_DIR}/TracyProfiler")
			target_compile_options(tracy-profiler PRIVATE -include stdint.h)
		endif ()
	else ()
		find_package(Tracy CONFIG QUIET)
		if (Tracy_FOUND)
			message(STATUS "Found Tracy")
		else ()
			message(FATAL_ERROR "Tracy was not found.\n\n"
				"Install Tracy or provide source with -DCEE_TRACY_SOURCE_DIR=/path/to/Tracy")
		endif ()
	endif ()
endif (USE_TRACY AND NOT TARGET Tracy::TracyClient)

add_subdirectory("${CMAKE_SOURCE_DIR}/external/glad"
	"${CMAKE_BINARY_DIR}/_deps/glad"
	EXCLUDE_FROM_ALL)

