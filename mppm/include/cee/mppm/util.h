/*
 * CeeHealth
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

#ifndef CEE_UTIL_H_
#define CEE_UTIL_H_

#include <cee/mppm/config.h>

#include <string>
#include <string_view>
#include <type_traits>

/*
 **************************************************
 ********** Useful macro helper functions *********
 **************************************************
 */
#define HAS_ARGS_IMPL(x, ...) x
#define HAS_ARGS(...) HAS_ARGS_IMPL(__VA_OPT__(1, ) 0)

#define COMPOSE_MACRO_IMPL(lhs, rhs) lhs##rhs
#define COMPOSE_MACRO(lhs, rhs) COMPOSE_MACRO_IMPL(lhs, rhs)

namespace cee {
namespace util {
	/*
	 **************************************************
	 ******** Useful type_traits like functions *******
	 **************************************************
	 */
	template<typename T>
	struct is_string : std::false_type {};

	template<typename charT>
	struct is_string<charT *> : std::bool_constant<std::is_same_v<std::remove_cv_t<charT>, char>> {};

	template<typename charT, std::size_t N>
	struct is_string<charT[N]> : std::bool_constant<std::is_same_v<std::remove_cv_t<charT>, char>> {};

	template<typename charT, typename Tr, typename Alloc>
	struct is_string<std::basic_string<charT, Tr, Alloc>> : std::true_type {};

	template<typename charT, typename Tr>
	struct is_string<std::basic_string_view<charT, Tr>> : std::true_type {};

	template<typename T>
	constexpr bool is_string_v = is_string<std::remove_cvref_t<T>>::value;

	/*
	 **************************************************
	 ************* Uncategorised functions ************
	 **************************************************
	 */
	template<typename E>
	requires std::is_enum_v<E>
	constexpr auto ToUnderlyingType(E e) {
		return static_cast<std::underlying_type_t<E>>(e);
	}
}
}

#endif

