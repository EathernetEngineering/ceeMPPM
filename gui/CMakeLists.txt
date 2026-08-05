# ceeGUI
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

project(ceeGUI LANGUAGES CXX VERSION 1.0.0)

find_package(Freetype REQUIRED)

list(APPEND GUI_PRIVATE_INCLUDEDIRS /usr/include ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
list(APPEND GUI_PUBLIC_INCLUDEDIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)
list(APPEND GUI_LIBRARYDIRS /usr/lib)
list(APPEND GUI_LIBRARIES glad glm::glm ceeCore ceeProfiler ceeMPPMPlatform Freetype::Freetype)

list(APPEND GUI_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/box.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/context.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/fontCache.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/fonts.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/gui.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/object.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/plot.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/shaders.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/text.cpp
)

add_library(ceeGUI ${GUI_SOURCES})

set_target_properties(ceeGUI PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON)

target_include_directories(ceeGUI PRIVATE ${GUI_PRIVATE_INCLUDEDIRS})
target_include_directories(ceeGUI PUBLIC ${GUI_PUBLIC_INCLUDEDIRS})
target_link_directories(ceeGUI PRIVATE ${GUI_LIBRARYDIRS})
target_link_libraries(ceeGUI PUBLIC ${GUI_LIBRARIES})

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
		target_compile_options(ceeGUI PRIVATE -Wall -Wextra -Wno-unused-parameter)
	if (CEE_USE_ASAN)
		target_compile_options(ceeGUI PRIVATE -fsanitize=address,leak)
		target_link_options(ceeGUI INTERFACE -fsanitize=address,leak)
	endif ()
else ()
	if (CEE_USE_ASAN)
		message(WARNING "Building ceeGUI with address sanitizer in non-debug build")
		target_compile_options(ceeGUI PRIVATE -fsanitize=address,leak)
		target_link_options(ceeGUI INTERFACE -fsanitize=address,leak)
	endif ()
endif ()

configure_file(config.h.in "${CMAKE_CURRENT_BINARY_DIR}/config.h" @ONLY)

