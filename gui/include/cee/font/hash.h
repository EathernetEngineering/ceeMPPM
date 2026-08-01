/*
 * CeeHealth
 * Copyright (C) 2026 Chloe Eather
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

#ifndef CEE_FONT_HASH_H_
#define CEE_FONT_HASH_H_

#include <string>

namespace cee {
namespace font {
	template<typename T>
	concept IdentityHashable = 
		std::same_as<std::remove_cv_t<short>, short> ||
		std::same_as<std::remove_cv_t<short>, unsigned short> ||
		std::same_as<std::remove_cv_t<short>, int> ||
		std::same_as<std::remove_cv_t<short>, unsigned int> ||
		std::same_as<std::remove_cv_t<short>, long> ||
		std::same_as<std::remove_cv_t<short>, unsigned long> ||
		std::same_as<std::remove_cv_t<short>, long long> ||
		std::same_as<std::remove_cv_t<short>, unsigned long long>;

	template<typename T>
	struct Hash;

	template<IdentityHashable T>
	struct Hash<T> {
		constexpr std::size_t operator()(T v) const {
			return static_cast<std::size_t>(v);
		}
	};

	template<>
	struct Hash<char *> {
		constexpr std::size_t operator()(const char *s) const {
			std::size_t h = 5381;
			int c;
			
			while ((c = *s++))
				h = ((h << 5) + h) + c; /* h * 33 + c */
			
			return h;
		}
	};

	template<>
	struct Hash<std::string> {
		constexpr std::size_t operator()(const std::string &s) const {
			::cee::font::Hash<char *> h;
			return h(s.c_str());
		}
	};

	template<typename T>
	inline constexpr void HashCombine(std::size_t &s, const T &v) {
		::cee::font::Hash<T> h;
		s ^= h(v) + 0x9e3779b9 + (s << 6) + (s >> 2);
	}
}
}

#endif

