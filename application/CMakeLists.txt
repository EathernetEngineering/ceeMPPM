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

list(APPEND MPPMA_PRIVATE_INCLUDEDIRS /usr/include)
list(APPEND MPPMA_LIBRARYDIRS /usr/lib)
list(APPEND MPPMA_LIBRARIES ceeMPPM)

add_executable(mppm application.cpp)

set_target_properties(mppm
	PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON)

target_include_directories(mppm PRIVATE ${MPPMA_PRIVATE_INCLUDEDIRS})
target_link_directories(mppm PRIVATE ${MPPMA_LIBRARYDIRS})
target_link_libraries(mppm PRIVATE ${MPPMA_LIBRARIES})

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
		target_compile_options(mppm PRIVATE -Wall -Wextra -Wno-unused-parameter)
	if (CEE_USE_ASAN)
		target_compile_options(mppm PRIVATE -fsanitize=address,leak)
		target_link_options(mppm INTERFACE -fsanitize=address,leak)
	endif ()
else ()
	if (CEE_USE_ASAN)
		message(WARNING "Building mppm with address sanitizer in non-debug build")
		target_compile_options(mppm PRIVATE -fsanitize=address,leak)
		target_link_options(mppm INTERFACE -fsanitize=address,leak)
	endif ()
endif ()

