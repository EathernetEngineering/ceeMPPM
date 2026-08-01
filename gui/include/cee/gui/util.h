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

#ifndef CEE_GUI_UTIL_H_
#define CEE_GUI_UTIL_H_

#include <cstdint>

namespace cee {
namespace gui {
	struct Utf8Result {
		uint32_t codepoint;
		uint8_t length;
		bool valid;
	};

	constexpr Utf8Result decodeUtf8(const char *text, std::size_t remaining) {
		if (remaining == 0)
			return { 0, 0, false };

		const auto *bytes = reinterpret_cast<const uint8_t *>(text);
		const uint8_t b0 = bytes[0];

		if (b0 <=0x7F)
			return { b0, 1, true };

		if ((b0 & 0xE0) == 0xC0) {
			if (remaining < 2 || (bytes[1] & 0xC0) != 0x80)
				return { 0xFFFD, 1, false };

			uint32_t cp = ((b0 & 0x1F) << 6) | (bytes[1] & 0x3F);

			if (cp < 0x80)
				return { 0xFFFD, 1, false };
			return { cp, 2, true };
		}

		if ((b0 & 0xF0) == 0xE0) {
			if (remaining < 3 || (bytes[1] & 0xC0) != 0x80 || (bytes[2] & 0xC0) != 0x80)
				return { 0xFFFD, 1, false };

			uint32_t cp = ((b0 & 0x0F) << 12) | ((bytes[1] & 0x3F) << 6) | (bytes[2] & 0x3F);
			if (cp < 0x800 || (cp >= 0xD800 && cp <= 0xDFFF))
					return { 0xFFFD, 1, false };
			return { cp, 3, true };
		}
		if ((b0 & 0xF8) == 0xF0) {
			if (remaining < 4 || (bytes[1] & 0xC0) != 0x90 || (bytes[2] & 0xC0) != 0x80 || (bytes[3] & 0xC0) != 0x80)
				return { 0xFFFD, 1, false };
			
			uint32_t cp = ((b0 & 0x7) << 18) | ((bytes[1] & 0x3F) << 12) | ((bytes[2] & 0x3F) << 6) | (bytes[3] & 0x3F);
			if (cp < 0x10000 || cp > 0x10FFFF)
				return { 0xFFFD, 1, false };
			return { cp, 4, true };
		}
		return { 0xFFFD, 1, false };
	}
}
}

#endif

