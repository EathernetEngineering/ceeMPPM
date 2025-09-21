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

option(BUILD_HAL_DRM "build HAL DRM graphics backend" ON)
option(BUILD_HAL_EGL_X11 "build HAL GLX graphics backend" OFF)
option(BUILD_HAL_GLX "build HAL GLX graphics backend" OFF)
option(BUILD_HAL_I2C "build HAL I2C backend for GLX" ON)
option(BUILD_HAL_I2C_SIM "build HAL simulated I2C backend for GLX" ON)

list(APPEND HAL_PRIVATE_INCLUDEDIRS /usr/include ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
list(APPEND HAL_PUBLIC_INCLUDEDIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)
list(APPEND HAL_LIBRARYDIRS /usr/lib)
list(APPEND HAL_LIBRARIES glad spdlog::spdlog)

if (BUILD_HAL_DRM)
	list(APPEND HAL_LIBRARIES drm gbm)
	list(APPEND HAL_PRIVATE_INCLUDEDIRS /usr/include/libdrm)
endif()
if (BUILD_HAL_GLX)
	list(APPEND HAL_LIBRARIES X11 xcb X11-xcb)
endif()
if (BUILD_HAL_EGL_X11)
	list(APPEND HAL_LIBRARIES X11)
endif()

list(APPEND HAL_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/gfx.c
	${CMAKE_CURRENT_SOURCE_DIR}/hal.c
	${CMAKE_CURRENT_SOURCE_DIR}/i2c.c
	${CMAKE_CURRENT_SOURCE_DIR}/log.cpp
)

list(APPEND HAL_PUBLIC_HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/include/cee/hal/gfx.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/cee/hal/hal.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/cee/hal/i2c.h
)

list(APPEND HAL_PRIVATE_HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/log.h
	${CMAKE_CURRENT_SOURCE_DIR}/util.h
)

if (BUILD_HAL_DRM)
	list(APPEND HAL_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/display.c
		${CMAKE_CURRENT_SOURCE_DIR}/gbmFramebuffer.c
		${CMAKE_CURRENT_SOURCE_DIR}/gfx_drm.c
	)
	list(APPEND HAL_PRIVATE_HEADERS
		${CMAKE_CURRENT_SOURCE_DIR}/display.h
		${CMAKE_CURRENT_SOURCE_DIR}/gbmFramebuffer.h
		${CMAKE_CURRENT_SOURCE_DIR}/gfx_drm.h
	)
endif()
if (BUILD_HAL_EGL_X11)
	list(APPEND HAL_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/gfx_egl_x.c
	)
	list(APPEND HAL_PRIVATE_HEADERS
		${CMAKE_CURRENT_SOURCE_DIR}/gfx_egl_x.h
	)
endif()
if (BUILD_HAL_GLX)
	list(APPEND HAL_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/gfx_glx.c
	)
	list(APPEND HAL_PRIVATE_HEADERS
		${CMAKE_CURRENT_SOURCE_DIR}/gfx_glx.h
	)
endif()

# option(BUILD_HAL_I2C "build HAL I2C backend for GLX" ON)
# option(BUILD_HAL_I2C_SIM "build HAL simulated I2C backend for GLX" ON)

if (BUILD_HAL_I2C)
	list(APPEND HAL_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/i2c_i2c.c
	)
	list(APPEND HAL_PRIVATE_HEADERS
		${CMAKE_CURRENT_SOURCE_DIR}/i2c_i2c.h
	)
endif()
if (BUILD_HAL_I2C_SIM)
	list(APPEND HAL_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/i2c_sim.c
	)
	list(APPEND HAL_PRIVATE_HEADERS
		${CMAKE_CURRENT_SOURCE_DIR}/i2c_sim.h
	)
endif()

add_library(ceeHAL ${HAL_SOURCES})

set_target_properties(ceeHAL
	PROPERTIES
	C_STANDARD 17
	C_STANDARD_REQUIRED ON
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	PUBLIC_HEADER "${HAL_PUBLIC_HEADERS}"
	PRIVATE_HEADER "${HAL_PRIVATE_HEADERS}")

target_include_directories(ceeHAL PRIVATE ${HAL_PRIVATE_INCLUDEDIRS})
target_include_directories(ceeHAL PUBLIC ${HAL_PUBLIC_INCLUDEDIRS})
target_link_directories(ceeHAL PRIVATE ${HAL_LIBRARYDIRS})
target_link_libraries(ceeHAL PUBLIC ${HAL_LIBRARIES})

configure_file(config.h.in "${CMAKE_CURRENT_BINARY_DIR}/config.h" @ONLY)

