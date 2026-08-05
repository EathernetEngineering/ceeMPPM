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

project(MultiPPM LANGUAGES C CXX VERSION 1.0.0)

option(CEE_BUILD_TESTS "Build tests" OFF)
option(CEE_USE_ASAN "Bulild with address sanitizer" OFF)
option(CEE_USE_TRACY "Bulild with profiling" OFF)
option(CEE_BUILD_TRACY_PROFILER "Bulild Tracy profiler" OFF)

if (CEE_USE_TRACY)
	add_compile_definitions(CEE_PROFILER_ENABLE=1)
	set(TRACY_ENABLE ON)
endif()

enable_testing()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(cmake/Dependencies.cmake)

add_subdirectory(ceeCore)
add_subdirectory(profiler)
add_subdirectory(platform)
add_subdirectory(gui)
add_subdirectory(mppm)
add_subdirectory(application)

if (CEE_BUILD_TESTS)
	add_subdirectory(tests)
endif()

