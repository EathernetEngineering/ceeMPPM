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

#ifndef CEE_FONT_CACHE_H_
#define CEE_FONT_CACHE_H_

#include <cee/font/hash.h>
#include <cee/font/types.h>

#include <functional>
#include <list>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace cee {
namespace font {
	constexpr int ATLAS_WIDTH  = 1024;
	constexpr int ATLAS_HEIGHT = 1024;

	struct GlyphKey {
		FontID fontId;
		int faceGlyphIndex;
		int sizePt;
		uint32_t flags;

		constexpr bool operator==(const GlyphKey &other) const {
			return fontId == other.fontId &&
				sizePt == other.sizePt &&
				flags == other.flags &&
				faceGlyphIndex == other.faceGlyphIndex;
		}
	};

	template<>
	struct Hash<GlyphKey> {
		constexpr std::size_t operator()(const GlyphKey &k) const {
			std::size_t s = 0;
			HashCombine(s, k.sizePt);
			HashCombine(s, k.flags);
			HashCombine(s, k.fontId);
			HashCombine(s, k.faceGlyphIndex);
			return s;
		}
	};

	struct GlyphBitmap {
		long width;
		long height;
		int pitch;
		const uint8_t *data;
	};

	struct AtlasData {
		long penX, penY;
		long lineHeight;
		bool valid;
		std::vector<uint8_t> data;
	};

	class GlyphCache {
	public:
		using GlyphKeyHash = Hash<GlyphKey>;

	public:
		static const Glyph *FindGlyph(const GlyphKey &key);
		static const Glyph &StoreGlyph(const Glyph &glyph, const GlyphKey &key);
		static const Glyph &StoreGlyph(Glyph &&glyph, const GlyphKey &key);
		static void ReleaseGlyph(const GlyphKey &key);

		static void invalidate_if(std::function<bool(const std::pair<GlyphKey, const Glyph> &g)> predicate);

	private:
		static std::mutex s_Mutex;
		static std::unordered_map<GlyphKey, Glyph, GlyphKeyHash> s_Glyphs;
	};

	class AtlasCache {
	public:
		struct StoreResult {
			AtlasPageID pageId;
			long atlasX;
			long width;
			long atlasY;
			long height;
		};

	private:
		using Entry = std::pair<AtlasPage, AtlasData>;

	public:
		static StoreResult StoreGlyph(const GlyphBitmap &bmp);

		static const AtlasPage &GetPage(AtlasPageID id);
		static void ErasePage(AtlasPageID id);

		static size_t GetPageCount() { return s_Entries.size(); }

	private:
		static Entry& AddPage();

	private:
		static std::mutex s_Mutex;
		static std::list<Entry> s_Entries;
		static AtlasPageID s_IDCounter;
	};
}
}

#endif

