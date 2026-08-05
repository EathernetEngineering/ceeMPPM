# ceeMPPM
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

option(CEE_ENABLE_ASSERTIONS "Allow assertions at runtime" OFF)
option(CEE_ENABLE_ASSERTIONS_RAISE "Allow assertions at runtime" OFF)

list(APPEND MPPM_PRIVATE_INCLUDEDIRS /usr/include ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
list(APPEND MPPM_PUBLIC_INCLUDEDIRS /usr/include/libdrm ${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_BINARY_DIR}/include)
list(APPEND MPPM_LIBRARYDIRS /usr/lib)
list(APPEND MPPM_LIBRARIES m pthread rt asound xkbcommon
	glad glm::glm spdlog::spdlog ceeMPPMPlatform ceeGUI ceeProfiler)

list(APPEND MPPM_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/input.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/mppm.cpp
)

add_library(ceeMPPM ${MPPM_SOURCES})

set_target_properties(ceeMPPM
	PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON)

target_include_directories(ceeMPPM PRIVATE ${MPPM_PRIVATE_INCLUDEDIRS})
target_include_directories(ceeMPPM PUBLIC ${MPPM_PUBLIC_INCLUDEDIRS})
target_link_directories(ceeMPPM PRIVATE ${MPPM_LIBRARYDIRS})
target_link_libraries(ceeMPPM PUBLIC ${MPPM_LIBRARIES})

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
		target_compile_options(ceeMPPM PRIVATE -Wall -Wextra -Wno-unused-parameter)
	if (CEE_USE_ASAN)
		target_compile_options(ceeMPPM PRIVATE -fsanitize=address,leak)
		target_link_options(ceeMPPM INTERFACE -fsanitize=address,leak)
	endif ()
else ()
	if (CEE_USE_ASAN)
		message(WARNING "Building ceeMPPM with address sanitizer in non-debug build")
		target_compile_options(ceeMPPM PRIVATE -fsanitize=address,leak)
		target_link_options(ceeMPPM INTERFACE -fsanitize=address,leak)
	endif ()
endif ()

configure_file(config.h.in "${CMAKE_CURRENT_BINARY_DIR}/include/cee/mppm/config.h" @ONLY)

