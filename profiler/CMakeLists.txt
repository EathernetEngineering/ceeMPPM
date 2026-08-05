# ceeProfiler
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

if (CEE_USE_TRACY)
	add_library(ceeProfiler INTERFACE)
	target_link_libraries(ceeProfiler INTERFACE Tracy::TracyClient)
	target_compile_definitions(ceeProfiler INTERFACE TRACY_ENABLE)
else()
	add_library(ceeProfiler INTERFACE)
endif()

target_include_directories(ceeProfiler INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

