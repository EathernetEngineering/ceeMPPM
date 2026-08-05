/*
 * ceeCore
 * Copyright (C) 2025 Chloe Eather
 *
 * This program is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CEE_ASSERT_H_
#define CEE_ASSERT_H_

#include <cee/core/config.h>

#if CEE_ENABLE_ASSERTIONS
	#include <spdlog/spdlog.h>
	#include <filesystem>
#endif
#if CEE_ENABLE_ASSERTIONS_RAISE
	#include <csignal>
#endif

/*
 **************************************************
 ****************** Assertions <3 *****************
 **************************************************
 */
#define HAS_ARGS_IMPL(x, ...) x
#define HAS_ARGS(...) HAS_ARGS_IMPL(__VA_OPT__(1, ) 0)

#define COMPOSE_MACRO_IMPL(lhs, rhs) lhs##rhs
#define COMPOSE_MACRO(lhs, rhs) COMPOSE_MACRO_IMPL(lhs, rhs)

#if CEE_ENABLE_ASSERTIONS_RAISE
	#if defined(__has_builtin) && __has_builtin(__builtin_debugtrap)
		#define CEE_DEBUG_BREAK() __builtin_debugtrap()
	#else
		#define CEE_DEBUG_BREAK() std::raise(SIGTRAP)
	#endif
#else // CEE_ENABLE_ASSERTIONS_RAISE
	#define CEE_DEBUG_BREAK() ((void)0)
#endif // CEE_ENABLE_ASSERTIONS_RAISE

#if CEE_ENABLE_ASSERTIONS
	#define CEE_ASSERT_IMPL_0(cond) do { \
		if (!(cond)) { \
			spdlog::critical("Assertion {} Failed: {}:{}", #cond, \
					 std::filesystem::path(__FILE__).filename().string(), \
					 __LINE__); \
			CEE_DEBUG_BREAK(); \
		} \
	} while (0)
	#define CEE_ASSERT_IMPL_1(cond, ...) do { \
		if (!(cond)) { \
			spdlog::critical("Assertion {} Failed: {}:{}", #cond, \
					 std::filesystem::path(__FILE__).filename().string(), \
					 __LINE__); \
			spdlog::critical(__VA_ARGS__); \
			CEE_DEBUG_BREAK(); \
		} \
	} while (0)
#else
	#define CEE_ASSERT_IMPL_0(...) ((void)(0))
	#define CEE_ASSERT_IMPL_1(...) ((void)(0))
#endif

#define CEE_ASSERT(cond, ...) \
	COMPOSE_MACRO(CEE_ASSERT_IMPL_, HAS_ARGS(__VA_ARGS__))((cond) __VA_OPT__(, ) __VA_ARGS__)

#endif

