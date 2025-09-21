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

if(BUILD_GL AND BUILD_GLES)
	message(SEND_ERROR "Cannot build both GL ES and OpenGL as they have conflicting symbols\n"
	                   "\tThis will cause link errors when compiled into an application")
endif()

if (BUILD_EGL)
	list(APPEND glad_SOURCES src/egl.c)
	list(APPEND glad_PUBLIC_HEADERS include/glad/egl.h include/EGL/eglplatform.h)
	list(APPEND glad_LIBRARIES EGL)
endif()
if (BUILD_GLES)
	list(APPEND glad_SOURCES src/gles2.c src/gles1.c)
	list(APPEND glad_PUBLIC_HEADERS include/glad/gles1.h include/glad/gles2.h)
	list(APPEND glad_LIBRARIES GLESv2)
endif()
if (BUILD_GL)
	list(APPEND glad_SOURCES src/gl.c src/glx.c)
	list(APPEND glad_PUBLIC_HEADERS include/glad/gl.h include/glad/glx.h)
	list(APPEND glad_LIBRARIES GL)
endif()
list(APPEND glad_PUBLIC_HEADERS include/KHR/khrplatform.h)



add_library(glad ${glad_SOURCES})

set_target_properties(glad
	PROPERTIES
	C_STANDARD 99
	C_STANDARD_REQUIRED ON
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	PUBLIC_HEADER "${glad_PUBLIC_HEADERS}")

target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(glad PUBLIC ${glad_LIBRARIES})

