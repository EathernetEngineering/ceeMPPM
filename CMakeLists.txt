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

project(MultiPPM LANGUAGES C CXX VERSION 1.0.0)

find_package(PkgConfig REQUIRED)

option(CEE_BUILD_TESTS "Build tests" OFF)
option(BUILD_EGL "Include EGL in final build" ON)
option(BUILD_GLES "Include GLES in final build" ON)
option(BUILD_GL "Include OpenGL in final build" OFF)

set(GTEST_HAS_ABSL ON)
set(ABSL_BUILD_TESTING OFF)
set(ABSL_ENABLE_INSTALL ON)
set(ABSL_USE_EXTERNAL_GOOGLETEST ON)
set(RE2_BUILD_TESTING OFF)
set(SPDLOG_BUILD_PIC ON)
set(SPDLOG_ENABLE_PCH ON)
set(SPDLOG_FMT_EXTERNAL ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(external/abseil-cpp)
add_subdirectory(external/re2)
add_subdirectory(external/googletest)
add_subdirectory(external/glad)
add_subdirectory(external/glm)
add_subdirectory(external/fmt)
add_subdirectory(external/spdlog)
add_subdirectory(gui)
add_subdirectory(hal)
add_subdirectory(mppm)
add_subdirectory(application)

enable_testing()

if (CEE_BUILD_TESTS)
	add_subdirectory(tests)
endif()

