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

project(ceeMPPMPlatform LANGUAGES CXX VERSION 1.0.0)

option(BUILD_PLATFORM_DRM "build DRM graphics backend" ON)
option(BUILD_PLATFORM_X11 "build X11 graphics backend" OFF)
option(BUILD_PLATFORM_I2C_HW "build I2C hardware backend" ON)
option(BUILD_PLATFORM_I2C_MOCK "build mock I2C backend" ON)

list(APPEND PLATFORM_PRIVATE_INCLUDEDIRS /usr/include ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
list(APPEND PLATFORM_PUBLIC_INCLUDEDIRS ${CMAKE_CURRENT_SOURCE_DIR}/include /usr/include/libdrm)
list(APPEND PLATFORM_LIBRARYDIRS /usr/lib)
list(APPEND PLATFORM_LIBRARIES glad ceeCore)

if (BUILD_PLATFORM_DRM)
	list(APPEND PLATFORM_LIBRARIES drm gbm)
endif()
if (BUILD_PLATFORM_X11)
	list(APPEND PLATFORM_LIBRARIES X11)
endif()

list(APPEND PLATFORM_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/gfx.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/i2c.cpp
)

if (BUILD_PLATFORM_DRM)
	list(APPEND PLATFORM_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/gfx_drm.cpp
	)
endif()
if (BUILD_PLATFORM_X11)
	list(APPEND PLATFORM_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/gfx_egl_x.cpp
	)
endif()

if (BUILD_PLATFORM_I2C)
	list(APPEND PLATFORM_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/i2c_hw.cpp
	)
endif()
if (BUILD_PLATFORM_I2C_MOCK)
	list(APPEND PLATFORM_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/i2c_mock.cpp
	)
endif()

add_library(ceeMPPMPlatform ${PLATFORM_SOURCES})

set_target_properties(ceeMPPMPlatform
	PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON)

target_include_directories(ceeMPPMPlatform PRIVATE ${PLATFORM_PRIVATE_INCLUDEDIRS})
target_include_directories(ceeMPPMPlatform PUBLIC ${PLATFORM_PUBLIC_INCLUDEDIRS})
target_link_directories(ceeMPPMPlatform PRIVATE ${PLATFORM_LIBRARYDIRS})
target_link_libraries(ceeMPPMPlatform PUBLIC ${PLATFORM_LIBRARIES})

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
		target_compile_options(ceeMPPMPlatform PRIVATE -Wall -Wextra -Wno-unused-parameter)
	if (CEE_USE_ASAN)
		target_compile_options(ceeMPPMPlatform PRIVATE -fsanitize=address,leak)
		target_link_options(ceeMPPMPlatform INTERFACE -fsanitize=address,leak)
	endif ()
else ()
	if (CEE_USE_ASAN)
		message(WARNING "Building ceeMPPMPlatform with address sanitizer in non-debug build")
		target_compile_options(ceeMPPMPlatform PRIVATE -fsanitize=address,leak)
		target_link_options(ceeMPPMPlatform INTERFACE -fsanitize=address,leak)
	endif ()
endif ()

configure_file(config.h.in "${CMAKE_CURRENT_BINARY_DIR}/config.h" @ONLY)

