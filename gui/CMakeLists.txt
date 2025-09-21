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

list(APPEND GUI_PRIVATE_INCLUDEDIRS /usr/include ${CMAKE_CURRENT_SOURCE_DIR})
list(APPEND GUI_PUBLIC_INCLUDEDIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)
list(APPEND GUI_LIBRARYDIRS /usr/lib)
list(APPEND GUI_LIBRARIES spdlog::spdlog glad)

list(APPEND GUI_SOURCES
	gui.cpp
)

list(APPEND GUI_PUBLIC_HEADERS
	include/cee/gui/cprocessordetection.h
	include/cee/gui/gridLayout.h
	include/cee/gui/gui.h
	include/cee/gui/layout.h
	include/cee/gui/object.h
	include/cee/gui/types.h
	include/cee/gui/widget.h
)

add_library(ceeGUI ${GUI_SOURCES})

set_target_properties(ceeGUI
	PROPERTIES
	C_STANDARD 17
	C_STANDARD_REQUIRED ON
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	PUBLIC_HEADER "${GUI_PUBLIC_HEADERS}")

target_include_directories(ceeGUI PRIVATE ${GUI_PRIVATE_INCLUDEDIRS})
target_include_directories(ceeGUI PUBLIC ${GUI_PUBLIC_INCLUDEDIRS})
target_link_directories(ceeGUI PRIVATE ${GUI_LIBRARYDIRS})
target_link_libraries(ceeGUI PUBLIC ${GUI_LIBRARIES})

