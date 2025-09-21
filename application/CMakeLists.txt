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

find_package(PkgConfig REQUIRED)

list(APPEND MPPMA_PRIVATE_INCLUDEDIRS /usr/include)
list(APPEND MPPMA_LIBRARYDIRS /usr/lib)
list(APPEND MPPMA_LIBRARIES ceeMPPM)

list(APPEND MPPMA_SOURCES
	application.cpp
)

add_executable(mppm ${MPPMA_SOURCES})

set_target_properties(mppm
	PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON)

target_include_directories(mppm PRIVATE ${MPPMA_PRIVATE_INCLUDEDIRS})
target_link_directories(mppm PRIVATE ${MPPMA_LIBRARYDIRS})
target_link_libraries(mppm PRIVATE ${MPPMA_LIBRARIES})

