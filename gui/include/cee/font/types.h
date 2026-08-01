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
#ifndef CEE_FONT_TYPES_H_
#define CEE_FONT_TYPES_H_

#include <cstdint>
#include <mutex>
#include <shared_mutex>

namespace cee {
namespace font {
	using FontID = int;
	using AtlasPageID = int;

	struct Glyph {
		uint32_t codepoint;
		long width, height;
		long bearingX, bearingY;
		long advance;
		long atlasX, atlasY;
		AtlasPageID atlasId;

		constexpr bool operator==(const Glyph &other) const {
			return codepoint == other.codepoint &&
				width == other.width &&
				height == other.height &&
				bearingX == other.bearingY &&
				bearingY == other.bearingY &&
				advance == other.advance &&
				atlasX == other.atlasX &&
				atlasY == other.atlasY &&
				atlasId == other.atlasId;
		}
	};

	struct AtlasPage {
		long width, height;
		int version;
		AtlasPageID id;

		mutable std::shared_mutex mutex;
		const uint8_t *data;

		AtlasPage(long w, long h, int ver, AtlasPageID id, const uint8_t *data)
		 : width(w), height(h), version(ver), id(id), data(data)
		{}
		AtlasPage(AtlasPage &&other) { *this = std::move(other); }

		AtlasPage &operator=(AtlasPage &&other) {
			std::unique_lock otherLock(other.mutex);

			width = other.width;
			height = other.height;
			version = other.version;
			id = other.id;
			data = other.data;

			return *this;
		}
	};
}
}

#endif

